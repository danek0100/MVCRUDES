#include    <include/database/controllers/postgresql/PostgreSqlDatabaseController.h>

PostgreSqlDatabaseController::PostgreSqlDatabaseController(std::shared_ptr<ITypesConverter> typesConverter,
                                                           IEntitiesStructureCache* structures) :
                                                           typesConverter(std::move(typesConverter)),
                                                           structures(structures) {}

PostgreSqlDatabaseController::~PostgreSqlDatabaseController() = default;


int PostgreSqlDatabaseController::createTable(const TableScheme& scheme) {
    LOG(LDEBUG, __FUNCTION__ << "() called. Table name: " << scheme.name);

    if (createEnums(scheme) != 0) {
        return -1;
    }

    std::stringstream query;
    query << "CREATE TABLE " << scheme.name << " (ID SERIAL, ";

    for (const auto& field : scheme.fields) {
        if (field.get(FIELD_NAME).asString() != KEY_FIELD) {
            query << generateFieldDefinition(field) << ", ";
        }
    }

    query << "PRIMARY KEY(ID));";

    auto poolClient = getSqlPoolClient();
    try {
        poolClient->get()->update(query.str());
    } catch (Exception* ex) {
        LOG(LERROR, __FUNCTION__ << "(). Creation table failed: " << ex->getText() << " query: " << query.str());
        delete ex;
        return -1;
    }

    if (
        addUniqueConstraints(scheme.uniqueFieldsSets, scheme.name) != 0 ||
        createIndexes(scheme.indexes, scheme.name) != 0 ||
        buildRelations(scheme.relations, scheme.name) != 0
        ) {
        return -1;
    }

    LOG(LDEBUG, __FUNCTION__ << "() table successfully created");
    return 0;
}

int PostgreSqlDatabaseController::createEnums(const TableScheme& scheme) {
    LOG(LDEBUG, __FUNCTION__ << "() called. Table name: " << scheme.name);
    std::set<std::string> typesQueries;
    for (const auto& field : scheme.fields) {
        if (!structures->isTypeEnum(field.get(FIELD_TYPE).asString())) {
            continue;
        }

        std::stringstream typeQuery;
        typeQuery << "CREATE TYPE " << field.get(FIELD_TYPE).asString() << " AS ENUM(";
        for (const std::string& enumValue : structures->getEnumValues(field.get(FIELD_TYPE).asString())) {
            typeQuery << "'" << enumValue << "',";
        }
        typeQuery.seekp(-1, typeQuery.cur);
        typeQuery << ");";
        typesQueries.insert(typeQuery.str());
    }

    auto poolClient = getSqlPoolClient();
    for (const std::string& typeQuery : typesQueries) {
        try {
            poolClient->get()->update(typeQuery);
        } catch (Exception* ex) {
            LOG(LERROR, __FUNCTION__ << "(). Creation enums failed: " << ex->getText() << " query: " << typeQuery);
            delete ex;
            return -1;
        }
    }
    LOG(LDEBUG, __FUNCTION__ << "() enums successfully created");
    return 0;
}

std::string PostgreSqlDatabaseController::generateFieldDefinition(const StructureField& field) {
    std::stringstream fieldDefinition;
    fieldDefinition << field.get(FIELD_NAME).asString() << " "
                    << typesConverter->getMappedType(field.get(FIELD_TYPE).asString(), !field.get(LENGTH_PROPERTY).isInt()
                                                                                       ? -1 : field.get(LENGTH_PROPERTY).asInt());

    if (field.get(DEFAULT_VALUE_PROPERTY).isInt() || field.get(DEFAULT_VALUE_PROPERTY).isString() || field.get(DEFAULT_VALUE_PROPERTY).isBool()) {
        fieldDefinition << " DEFAULT "
                        << typesConverter->adaptValueToType(field.get(FIELD_TYPE).asString(), field.get(DEFAULT_VALUE_PROPERTY));
    }

    if (field.get(CAN_BE_EMPTY_PROPERTY).isBool() && field.get(CAN_BE_EMPTY_PROPERTY).asBool() == false) {
        fieldDefinition << " NOT NULL";
    }
    return fieldDefinition.str();
}

int PostgreSqlDatabaseController::addUniqueConstraints(const std::unordered_map<std::string, std::vector<std::string>>& uniqueFieldsSets, const std::string& tableName) {
    LOG(LDEBUG, __FUNCTION__ << "() called. Table name: " << tableName);
    auto poolClient = getSqlPoolClient();

    for (const auto& fieldsSet : uniqueFieldsSets) {
        if (fieldsSet.second.empty()) continue;

        std::string fieldsSetString = StringTools::concat(fieldsSet.second.begin(), fieldsSet.second.end(), ',');

        std::stringstream uniqueQuery;
        uniqueQuery << "ALTER TABLE " << tableName <<
                    " ADD CONSTRAINT " << fieldsSet.first << " UNIQUE (" << fieldsSetString << ");";

        try {
            poolClient->get()->update(uniqueQuery.str());
        } catch (Exception* ex) {
            LOG(LERROR, __FUNCTION__ << "(). Creation unique constraint failed: " << ex->getText() << " query: " << uniqueQuery.str());
            delete ex;
            return -1;
        }
    }
    return 0;
}

int PostgreSqlDatabaseController::createIndexes(const std::unordered_map<std::string, std::vector<std::string>>& indexes, const std::string& tableName) {
    LOG(LDEBUG, __FUNCTION__ << "() called. Table name: " << tableName);
    auto poolClient = getSqlPoolClient();

    for (const auto& index : indexes) {
        if (index.second.empty()) continue;

        std::string fieldsSetString = StringTools::concat(index.second.begin(), index.second.end(), ',');

        std::stringstream indexQuery;
        indexQuery << "CREATE INDEX " << index.first << " ON " << tableName << "(" << fieldsSetString << ");";
        try {
            poolClient->get()->update(indexQuery.str());
        } catch (Exception* ex) {
            LOG(LERROR, __FUNCTION__ << "(). Creation index failed: " << ex->getText() << " query: " << indexQuery.str());
            delete ex;
            return -1;
        }
    }
    return 0;
}

int PostgreSqlDatabaseController::buildRelations(const std::unordered_map<std::string, Relation>& relations, const std::string& tableName) {
    LOG(LDEBUG, __FUNCTION__ << "() called. Table name: " << tableName);
    auto poolClient = getSqlPoolClient();

    for (const auto& relation : relations) {

        // Грязный хак, чтобы избежать SEH
        auto localFields = relation.second.getLocalFields();
        auto targetFields = relation.second.getTargetFields();

        std::string localFieldsSetString = StringTools::concat(localFields.begin(), localFields.end(), ',');

        std::string targetFieldsSetString = StringTools::concat(targetFields.begin(), targetFields.end(), ',');

        std::stringstream indexQuery;
        indexQuery << "ALTER TABLE " << tableName << " ADD CONSTRAINT " << relation.first
                    << " FOREIGN KEY (" << localFieldsSetString << ") "
                    << "REFERENCES " << relation.second.getTargetEntity() << "(" << targetFieldsSetString << ") "
                    << "ON DELETE " << Relation::relationTypeToStr(relation.second.getOnDelete())
                    << " ON UPDATE " << Relation::relationTypeToStr(relation.second.getOnUpdate()) << ";";

        try {
            poolClient->get()->update(indexQuery.str());
        } catch (Exception* ex) {
            LOG(LERROR, __FUNCTION__ << "(). Creation relation failed: " << ex->getText() << " query: " << indexQuery.str());
            delete ex;
            return -1;
        }
    }
    return 0;
}

bool PostgreSqlDatabaseController::tableExists(const std::string& tableName) {
    std::string query = "SELECT EXISTS (SELECT FROM pg_tables WHERE schemaname = 'public' AND tablename = '" + StringTools::toLower(tableName) + "');";
    auto poolClient = getSqlPoolClient();
    try {
        auto result = poolClient->get()->query(query);
        if (result->next() && result->getInt("EXISTS") == 1) {
            LOG(LTRACE, __FUNCTION__ << "(). Table " << tableName << " exists.");
            return true;
        }
        LOG(LTRACE, __FUNCTION__ << "(). Table " << tableName << " doesn't exists.");
    } catch (Exception* ex) {
        LOG(LERROR, __FUNCTION__ << "(). Table existence check failed: " << ex->getText() << " query: " << query);
        delete ex;
    }
    return false;
}

int PostgreSqlDatabaseController::dropTable(const TableScheme& scheme) {
    LOG(LDEBUG, __FUNCTION__ << "(). Entry point.")

    if (!tableExists(scheme.name)) {
        LOG(LDEBUG, __FUNCTION__ << "(). Table " << scheme.name << " does not exist");
        return 0;
    }

    if (dropEnums(scheme) != 0) {
        LOG(LERROR, __FUNCTION__ << "(). Problem with dropping enums for " << scheme.name);
        return -1;
    }

    std::stringstream query;
    query << "DROP TABLE IF EXISTS " << scheme.name << ";";

    auto poolClient = getSqlPoolClient();
    try {
        poolClient->get()->update(query.str());
        LOG(LDEBUG, __FUNCTION__ << "(). Table dropped successfully: " << scheme.name);
    } catch (Exception* ex) {
        LOG(LERROR, __FUNCTION__ << "(). Drop failed: " << ex->getText() << " query: " << query.str());
        delete ex;
        return -2;
    }

    LOG(LDEBUG, __FUNCTION__ << "(). End point.")
    return 0;
}

int PostgreSqlDatabaseController::dropEnums(const TableScheme& scheme) {
    LOG(LTRACE, __FUNCTION__ << "() entry point");
    std::set<std::string> typesQueries;
    for (const auto& field : scheme.fields) {
        if (!structures->isTypeEnum(field.get(FIELD_TYPE).asString())) {
            continue;
        }
        typesQueries.insert("DROP TYPE IF EXISTS " + field.get(FIELD_TYPE).asString() + " CASCADE;");
    }

    auto poolClient = getSqlPoolClient();
    for (const std::string& typeQuery : typesQueries) {
        try {
            poolClient->get()->update(typeQuery);
        } catch (Exception* ex) {
            LOG(LERROR, __FUNCTION__ << "(). Drop enum failed: " << ex->getText() << " query: " << typeQuery);
            delete ex;
            return -1;
        }
    }
    LOG(LTRACE, __FUNCTION__ << "() end point");
    return 0;
}

void PostgreSqlDatabaseController::resetDatabases() {
    TRACE(__FUNCTION__ << "(). Entry point.")

    try {
        SqlPoolClient poolClient;

        /*
         * Специальный BOOLEAN-тип данных для совместимости с TINYINT MYSQL.
         * В качестве boolean-колонок должен использоваться именно он.
         */
        poolClient.get()->update("DROP DOMAIN IF EXISTS mysql_bool CASCADE;");
        poolClient.get()->update("CREATE DOMAIN mysql_bool AS smallint;");
        TRACE(__FUNCTION__ << "(). Databases was reset successfully");
    } catch (Exception* ex) {
        TRACE(__FUNCTION__ << "(). Drop failed: " << ex->getText());
        delete ex;
    }

    TRACE(__FUNCTION__ << "(). End point.")
}

void PostgreSqlDatabaseController::createDatabase(const std::string& dbName) {
    TRACE(__FUNCTION__ << "(). Entry point.")
    // TODO
    TRACE(__FUNCTION__ << "(). End point.")
}

void PostgreSqlDatabaseController::dropDatabase(const std::string& dbName) {
    TRACE(__FUNCTION__ << "(). Entry point.")
    // TODO
    TRACE(__FUNCTION__ << "(). End point.")
}

int PostgreSqlDatabaseController::getVersionFromVersionTable(const std::string& tableName) {
    LOG(LTRACE, __FUNCTION__ << "(). entry point");
    std::string query = "SELECT VERSION FROM " + VERSIONS_TABLE_NAME + " WHERE TABLENAME='" + tableName + "';";
    auto poolClient = getSqlPoolClient();
    try {
        auto result = poolClient->get()->query(query);
        if (result->next()) {
            int version = result->getInt(VERSION_PROPERTY);
            LOG(LTRACE, __FUNCTION__ << "(). Version found: " << version);
            return version;
        }
    } catch (Exception* ex) {
        LOG(LERROR, __FUNCTION__ << "(). Select failed: " << ex->getText() << " query: " << query);
        delete ex;
    }
    LOG(LTRACE, __FUNCTION__ << "(). Version not found");
    return -2;
}

int PostgreSqlDatabaseController::getEntityVersion(const std::string& tableName) {
    LOG(LDEBUG, __FUNCTION__ << "(). Entry point.");

    int version = getVersionFromVersionTable(tableName);
    if (version != -2) {
        LOG(LDEBUG, __FUNCTION__ << "() found version: " << version);
        return version;
    }

    if (tableExists(tableName)) {
        LOG(LDEBUG, __FUNCTION__ << "(). Table exists but no version: " << tableName);
        return -1;
    }
    LOG(LDEBUG, __FUNCTION__ << "(). Table does not exist: " << tableName);
    return -2;
}

int PostgreSqlDatabaseController::upsertTableVersion(const std::string& tableName, int version) {
    LOG(LDEBUG, __FUNCTION__ << "(). Entry point.");

    std::ostringstream query;
    query << "INSERT INTO " << StringTools::toLower(VERSIONS_TABLE_NAME)
          << " (TABLENAME, VERSION) VALUES ('"
          << tableName << "', " << version << ")"
          << " ON CONFLICT (TABLENAME) DO UPDATE SET VERSION = EXCLUDED.VERSION;";

    try {
        getSqlPoolClient()->get()->update(query.str());
    } catch (Exception* ex) {
        LOG(LERROR, __FUNCTION__ << "(). Insert/Update failed: " << ex->getText() << " for table: " << tableName);
        delete ex;
        return -1;
    }
    LOG(LDEBUG, __FUNCTION__ << "(). Success for table: " << tableName);
    return 0;
}
