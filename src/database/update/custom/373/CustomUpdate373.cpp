#include    "CustomUpdate373.h"
#include    "./EmpRtData/EmpService.h"
#include    <algorithm>

int CustomUpdate373::update() {
    TRACE( __FUNCTION__ << "() Update 373 started.");
    std::vector<std::string> queries;
    auto dbType = DbUsersConfig::get()->getDbServerType();

    // Приводим имена ключей к стандарту
    if (dbType == DBSERVERTYPE_POSTGRES) {
        queries.emplace_back("ALTER TABLE PARAMI DROP CONSTRAINT IF EXISTS unique_name_parami;");
        queries.emplace_back("ALTER TABLE PARAMB DROP CONSTRAINT IF EXISTS unique_name_paramb;");
    } else if (dbType == DBSERVERTYPE_INTERNAL || dbType == DBSERVERTYPE_MYSQL) {
        queries.emplace_back("ALTER TABLE PARAMI DROP INDEX NAME;");
        queries.emplace_back("ALTER TABLE PARAMB DROP INDEX NAME;");
        queries.emplace_back("UPDATE PARAMB SET PARAMVALUE = '' WHERE PARAMVALUE IS NULL;");
    }

    // Добавляем колонку пинкодов в таблицу персонала
    if (dbType == DBSERVERTYPE_POSTGRES) {
        queries.emplace_back("ALTER TABLE PERSONAL ADD COLUMN IF NOT EXISTS PIN_CODE varchar(4) DEFAULT '0000';");
        queries.emplace_back("UPDATE PERSONAL SET PIN_CODE = NULL WHERE TYPE = 'DEP';");
    } else if (dbType == DBSERVERTYPE_INTERNAL || dbType == DBSERVERTYPE_MYSQL) {
        queries.emplace_back("ALTER TABLE `PERSONAL` ADD COLUMN IF NOT EXISTS `PIN_CODE` varchar(4) DEFAULT '0000';");
        queries.emplace_back("UPDATE `PERSONAL` SET `PIN_CODE` = NULL WHERE `TYPE` = 'DEP';");
    }

    // Add columns GID and PARENT_GID in PERSONAL, GID - global id
    if (dbType == DBSERVERTYPE_POSTGRES) {
        queries.emplace_back("ALTER TABLE PERSONAL ADD COLUMN IF NOT EXISTS GID varchar(128);");
        queries.emplace_back("ALTER TABLE PERSONAL ADD COLUMN IF NOT EXISTS PARENT_GID varchar(128);");
        queries.emplace_back("CREATE INDEX PARENT_GID_IDX ON PERSONAL(PARENT_GID);");
        queries.emplace_back("ALTER TABLE PERSONAL ADD CONSTRAINT personal_gid_key UNIQUE (GID);");
    } else if (dbType == DBSERVERTYPE_INTERNAL || dbType == DBSERVERTYPE_MYSQL) {
        queries.emplace_back("ALTER TABLE `PERSONAL` ADD COLUMN IF NOT EXISTS `GID` varchar(128);");
        queries.emplace_back("ALTER TABLE `PERSONAL` ADD COLUMN IF NOT EXISTS `PARENT_GID` varchar(128);");
        queries.emplace_back("CREATE INDEX PARENT_GID_IDX ON `PERSONAL`(`PARENT_GID`);");
        queries.emplace_back("ALTER TABLE `PERSONAL` ADD CONSTRAINT personal_gid_key UNIQUE (`GID`);");
    }

    // В 373 обновлении создаём таблицу версий сущностей
    DatabaseManager::getSingletonInstance()->createVersionsTable();

    // Добавляем в таблицу ACCESSRULES колонку REDEFINITIONTYPE
    if (dbType == DBSERVERTYPE_POSTGRES) {
        queries.emplace_back("DROP TYPE IF EXISTS REDEFINITIONTYPE_TYPE CASCADE;");
        queries.emplace_back("CREATE TYPE REDEFINITIONTYPE_TYPE AS ENUM('RULES', 'INTERVALS','INTERVALS_AND_RULES');");
        queries.emplace_back("ALTER TABLE ACCESSRULES ADD COLUMN IF NOT EXISTS REDEFINITIONTYPE REDEFINITIONTYPE_TYPE default 'INTERVALS_AND_RULES';");
    } else if (dbType == DBSERVERTYPE_INTERNAL || dbType == DBSERVERTYPE_MYSQL) {
        queries.emplace_back("ALTER TABLE ACCESSRULES ADD COLUMN IF NOT EXISTS REDEFINITIONTYPE ENUM('RULES', 'INTERVALS','INTERVALS_AND_RULES') default 'INTERVALS_AND_RULES';");
    }

    // Заменяем в таблице ACCESSRULES колонки COND_TYPE, COND_WHERE, COND_FOR на одну CONDITIONS, перезаписывая данные
    try {
        TRACE(__FUNCTION__ << "(): " << "update conditions in ACCESSRULES:" << " entry point");
        SqlPoolClient sqlClient;
        SqlResult sqlResult;
        if (dbType == DBSERVERTYPE_POSTGRES) {
            queries.emplace_back(
                    "ALTER TABLE ACCESSRULES ADD COLUMN IF NOT EXISTS CONDITIONS VARCHAR(4000) DEFAULT '{\"type\":\"ALWAYS\"}';");
        } else if (dbType == DBSERVERTYPE_INTERNAL || dbType == DBSERVERTYPE_MYSQL) {
            queries.emplace_back(
                    "ALTER TABLE ACCESSRULES ADD COLUMN IF NOT EXISTS CONDITIONS VARCHAR(4000) CHARACTER SET UTF8 DEFAULT '{\"type\":\"ALWAYS\"}';");
        }

        Json::Value condition;
        Json::FastWriter jsonWriter;
        std::string str;
        sqlClient.get()->query("SELECT ID,COND_TYPE,COND_SEARCH_WHERE,COND_SEARCH_FOR FROM ACCESSRULES", sqlResult);
        while( sqlResult.next() ){
            condition = Json::Value(Json::objectValue);
            parseAccessRuleCond(condition, sqlResult);
            str = jsonWriter.write(condition);
            char query[256];
            sprintf_safe(query,sizeof(query),"UPDATE ACCESSRULES SET CONDITIONS='%s' where id=%d", str.c_str(), sqlResult.getInt("ID"));
            queries.emplace_back(query);
        }

        queries.emplace_back("ALTER TABLE ACCESSRULES DROP COLUMN IF EXISTS COND_SEARCH_WHERE;");
        queries.emplace_back("ALTER TABLE ACCESSRULES DROP COLUMN IF EXISTS COND_TYPE;");
        queries.emplace_back("ALTER TABLE ACCESSRULES DROP COLUMN IF EXISTS COND_SEARCH_FOR;");

        TRACE(__FUNCTION__ << "(): " << "update conditions in ACCESSRULES:" << " end point");
    }
    catch( Exception *pxE )
    {
        TRACE( "Failed to update conditions for ACCESSRULES table: " << pxE->getText() );
        delete pxE;
        return -1;
    }

   // device_security_data
   if (dbType == DBSERVERTYPE_POSTGRES) {
       queries.emplace_back("ALTER TABLE DEVICES_SECURITY_DATA DROP CONSTRAINT devices_security_data_pkey;");
       queries.emplace_back("ALTER TABLE DEVICES_SECURITY_DATA ADD ID SERIAL PRIMARY KEY;");
       queries.emplace_back("ALTER TABLE DEVICES_SECURITY_DATA ADD CONSTRAINT devices_security_data_serial_number_key UNIQUE(SERIAL_NUMBER);");
   } else if (dbType == DBSERVERTYPE_INTERNAL || dbType == DBSERVERTYPE_MYSQL) {
       queries.emplace_back("ALTER TABLE DEVICES_SECURITY_DATA DROP PRIMARY KEY;");
       queries.emplace_back("ALTER TABLE DEVICES_SECURITY_DATA ADD `ID` INT AUTO_INCREMENT PRIMARY KEY FIRST;");
       queries.emplace_back("ALTER TABLE DEVICES_SECURITY_DATA ADD UNIQUE devices_security_data_serial_number_key (SERIAL_NUMBER);");
   }

   queries.emplace_back("UPDATE PARAMB SET PARAMVALUE='" + std::string("тест") + "' WHERE NAME='" + std::string("DEBUG_TESTSTR") + "'");

   // удаление старых сущностей SOFTDEV-2416, SOFTDEV-3094
    if (dbType == DBSERVERTYPE_POSTGRES) {
        queries.emplace_back("DROP TABLE IF EXISTS DEPLOYMENTS;");
        queries.emplace_back("DROP TABLE IF EXISTS SYNC_EVENTS_JOURNAL;");
        queries.emplace_back("DROP TABLE IF EXISTS REPLICATION_USER;");
        queries.emplace_back("DROP TABLE IF EXISTS REMOTE_EVENTS;");
        queries.emplace_back("DROP TABLE IF EXISTS REMOTE_ACCESS_POINTS;");
        queries.emplace_back("ALTER TABLE ACCESSRULES DROP COLUMN IF EXISTS DEPLOYMENT_ID CASCADE");
        queries.emplace_back("ALTER TABLE ACCESSRULES DROP COLUMN IF EXISTS DEPLOYMENT_ENTITY_ID CASCADE");
        queries.emplace_back("ALTER TABLE PERSONAL DROP COLUMN IF EXISTS DEPLOYMENT_ID CASCADE");
        queries.emplace_back("ALTER TABLE PERSONAL DROP COLUMN IF EXISTS DEPLOYMENT_ENTITY_ID CASCADE");
        queries.emplace_back("DELETE FROM MISC_QUAR_RULES WHERE GLOBAL_QUARANTINE_ID IS NOT NULL ");
        queries.emplace_back("ALTER TABLE MISC_QUAR_RULES DROP COLUMN IF EXISTS GLOBAL_QUARANTINE_ID CASCADE");
        queries.emplace_back("ALTER TABLE DEVICES DROP COLUMN IF EXISTS AREA CASCADE");
        queries.emplace_back("ALTER TABLE LOGS DROP COLUMN IF EXISTS AREA CASCADE");
    } else if (dbType == DBSERVERTYPE_INTERNAL || dbType == DBSERVERTYPE_MYSQL) {
        queries.emplace_back("DROP TABLE IF EXISTS `DEPLOYMENTS`;");
        queries.emplace_back("DROP TABLE IF EXISTS `SYNC_EVENTS_JOURNAL`;");
        queries.emplace_back("DROP TABLE IF EXISTS `REPLICATION_USER`;");
        queries.emplace_back("DROP TABLE IF EXISTS `REMOTE_EVENTS`;");
        queries.emplace_back("DROP TABLE IF EXISTS `REMOTE_ACCESS_POINTS`;");
        queries.emplace_back("ALTER TABLE ACCESSRULES DROP COLUMN IF EXISTS `DEPLOYMENT_ID` CASCADE");
        queries.emplace_back("ALTER TABLE ACCESSRULES DROP COLUMN IF EXISTS `DEPLOYMENT_ENTITY_ID` CASCADE");
        queries.emplace_back("ALTER TABLE PERSONAL DROP COLUMN IF EXISTS `DEPLOYMENT_ID` CASCADE");
        queries.emplace_back("ALTER TABLE PERSONAL DROP COLUMN IF EXISTS `DEPLOYMENT_ENTITY_ID` CASCADE");
        queries.emplace_back("DELETE FROM MISC_QUAR_RULES WHERE `GLOBAL_QUARANTINE_ID` IS NOT NULL ");
        queries.emplace_back("ALTER TABLE MISC_QUAR_RULES DROP COLUMN IF EXISTS `GLOBAL_QUARANTINE_ID` CASCADE");
        queries.emplace_back("ALTER TABLE DEVICES DROP COLUMN IF EXISTS `AREA` CASCADE");
        queries.emplace_back("ALTER TABLE `TC-DB-LOG`.LOGS DROP COLUMN IF EXISTS `AREA` CASCADE");
    }

    queries.emplace_back("ALTER TABLE CERTIFICATES RENAME COLUMN CERT_ID TO ID");
    queries.emplace_back("ALTER TABLE LOCALIZATION_LOOKUP RENAME COLUMN ID TO ID_NAME");

    if (dbType == DBSERVERTYPE_POSTGRES) {
        queries.emplace_back("DROP TABLE IF EXISTS ZONES_DEPARTMENT_BINDINGS;");
    } else if (dbType == DBSERVERTYPE_INTERNAL || dbType == DBSERVERTYPE_MYSQL) {
        queries.emplace_back("DROP TABLE IF EXISTS `ZONES_DEPARTMENT_BINDINGS`;");
    }


    // Добавляем колонку для вложений в таблицу EMAILQUEUE
    if (dbType == DBSERVERTYPE_POSTGRES) {
        queries.emplace_back("ALTER TABLE EMAILQUEUE ADD COLUMN IF NOT EXISTS ATTACHMENT TEXT;");
    } else if (dbType == DBSERVERTYPE_INTERNAL || dbType == DBSERVERTYPE_MYSQL) {
        queries.emplace_back("ALTER TABLE `EMAILQUEUE` ADD COLUMN IF NOT EXISTS `ATTACHMENT` MEDIUMTEXT CHARACTER SET UTF8;");
    }

    SqlPoolClient sqlPool;
    for (const auto& query : queries) {
        try {
            sqlPool.get()->update(query);
        } catch (Exception* ex) {
            if (query == "ALTER TABLE PARAMI DROP INDEX NAME;" || query == "ALTER TABLE PARAMB DROP INDEX NAME;") {
                // Игнорируем, так как не проверяем был ли такой индекс вообще
                delete ex;
            }
            else {
                TRACE(__FUNCTION__ << "(). Cannot make update 373. " << ex->getText() << " query: '" << query << "'")
                delete ex;
                return -1;
            }
        }
    }

    // Создадим новые ключи и приведём таблицы к нужной форме
    DbStructureUpdater::getSingletonInstance()->updateStructuresForCustom(373);

    try
    {
        sqlPool.get()->update("UPDATE ZONES SET EVACREP = 'EVACUATION' WHERE EVACREP_EVAC = 1");
        sqlPool.get()->update("UPDATE ZONES SET EVACREP = 'WORK' WHERE EVACREP_WRK = 1");

        // Генерируем значения пинкодов для персонала

        SqlResult xSqlRes;
        sqlPool.get()->query( "SELECT ID,CODEKEY FROM PERSONAL WHERE TYPE!='DEP'", xSqlRes );
        while ( xSqlRes.next() )
        {
            int empId = xSqlRes.getInt("ID");
            int newPin;
            if (xSqlRes.isDataAvailable("CODEKEY")){
                //генерируем пин на основе первого пропуска
                unsigned char bKey[8];
                xSqlRes.getRaw("CODEKEY",bKey,8);
                newPin = EmpService::getPin(bKey);
            }
            else {
                //генерируем случайный пин, если нет пропуска
                newPin = EmpService::generateRandomPin();
            }
            char strPin[64];
            sprintf_safe(strPin,sizeof(strPin),"%04d", newPin);
            char query[256];
            sprintf_safe( query, sizeof(query),"UPDATE PERSONAL SET PIN_CODE='%s' WHERE ID=%d",strPin, empId );
            SqlPoolClient sqlPoolUpd;
            sqlPoolUpd.get()->update(query);
        }
    }
    catch( Exception *pxE )
    {
        TRACE( "Failed to get data from the DB: " << pxE->getText() );
        delete pxE;
        return -1;
    }

    updatePayMenuItems();

    // Обновляем кастомную версию
    auto& crudController = DatabaseManager::getCrudController();
    Json::Value jsonFilter;
    jsonFilter[JsonEntityFilter::FIELD_NAME] = "NAME";
    jsonFilter[JsonEntityFilter::TYPE] = JsonEntityFilter::EQ;
    jsonFilter[JsonEntityFilter::VALUE] = "DBVER";
    auto version = crudController->loadEntities("PARAMI", jsonFilter);

    if (!version.empty()) {
        version.front().value["PARAMVALUE"] = 373;
        crudController->updateEntities(version);
    } else {
        TRACE( __FUNCTION__ << "() WARNING! DBVER is absent.");
    }

    TRACE( __FUNCTION__ << "() Update 373 finished.");
    return 0;
}

void CustomUpdate373::parseAccessRuleCond(Json::Value &pValue, SqlResult &sqlResult) {
    std::string type = sqlResult.getString("COND_TYPE");
    if(type != "ALWAYS" && type != "NEVER"){
        pValue["fieldName"] = sqlResult.getString("COND_SEARCH_WHERE");
        if(type!= "IF_EMPTY" && type != "IF_NOT_EMPTY"){
            pValue["value"] = sqlResult.getString("COND_SEARCH_FOR");
        }
    }
    if( type == "IF_LT" || type == "IF_LE" || type == "IF_GT" || type == "IF_GE" || type == "IF_EQUALS" ) {
        pValue["type"] = type.substr(3, 2);
    } else if( type == "IF_NOT_EQUAL"){
        pValue["type"] = "NE";
    } else if( type == "IF_CONTAINS"){
        pValue["type"] = "CT";
    } else if( type == "IF_NOT_CONTAIN"){
        pValue["type"] = "NC";
    } else if( type == "IF_EMPTY"){
        pValue["type"] = "EP";
    } else if( type == "IF_NOT_EMPTY"){
        pValue["type"] = "NP";
    } else {
        pValue["type"] = type;
    }
}

void CustomUpdate373::updatePayMenuItems() {

    // update costParam1
    std::string query = "SELECT ID, COST_PARAM1 FROM PAYMENUITEMS WHERE COST_TYPE='SIDEPARAM'";
    SqlResult sqlResult;
    SqlPoolClient sqlPool;
    sqlPool.get()->query(query, sqlResult);

    while (sqlResult.next()) {
        SqlPoolClient sqlPoolInner;
        int menuItemId = sqlResult.getInt("ID");
        std::string costParam1 = sqlResult.getString("COST_PARAM1");

        query = "SELECT PARAM_IDX FROM SIDEPARAMTYPES WHERE TABLE_ID=0 AND NAME='" + StringTools::escapeSql(costParam1) + "' ORDER BY ID";
        SqlResult result;
        sqlPoolInner.get()->query(query, result);

        if (result.next()) {
            int paramIndex = result.getInt("PARAM_IDX");
            query = "UPDATE PAYMENUITEMS SET COST_PARAM1=" + std::to_string(paramIndex) + " WHERE ID=" + std::to_string(menuItemId);
        } else {
            query = "UPDATE PAYMENUITEMS SET COST_PARAM1=NULL WHERE ID=" + std::to_string(menuItemId);
        }
        sqlPoolInner.get()->update(query);
    }

    // update RestrCountParam1
    std::string queryRestr = "SELECT ID, RESTR_COUNT_PARAM1 FROM PAYMENUITEMS WHERE RESTR_COUNT_TYPE='SIDEPARAM'";
    SqlResult sqlResultRestr;
    sqlPool.get()->query(queryRestr, sqlResultRestr);

    while (sqlResultRestr.next()) {
        SqlPoolClient sqlPoolInner;
        int menuItemId = sqlResultRestr.getInt("ID");
        std::string restrCountParam1 = sqlResultRestr.getString("RESTR_COUNT_PARAM1");

        query = "SELECT PARAM_IDX FROM SIDEPARAMTYPES WHERE TABLE_ID=0 AND NAME='" + StringTools::escapeSql(restrCountParam1) + "' ORDER BY ID";
        SqlResult result;
        sqlPoolInner.get()->query(query, result);

        if (result.next()) {
            int paramIndex = result.getInt("PARAM_IDX");
            query = "UPDATE PAYMENUITEMS SET RESTR_COUNT_PARAM1=" + std::to_string(paramIndex) + " WHERE ID=" + std::to_string(menuItemId);
        } else {
            query = "UPDATE PAYMENUITEMS SET RESTR_COUNT_PARAM1=NULL WHERE ID=" + std::to_string(menuItemId);
        }
        sqlPoolInner.get()->update(query);
    }
}
