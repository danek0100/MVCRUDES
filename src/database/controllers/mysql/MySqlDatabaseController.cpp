#include    <include/database/controllers/mysql/MySqlDatabaseController.h>

static const int MAX_ROW_SIZE = 65536;  // Максимальный размер строки в MySQL

MySqlDatabaseController::MySqlDatabaseController(std::shared_ptr<ITypesConverter> typesConverter)
                                                                        : typesConverter(std::move(typesConverter)) {}

MySqlDatabaseController::~MySqlDatabaseController() = default;

int MySqlDatabaseController::createTable(const TableScheme& scheme) {
    LOG(LDEBUG, __FUNCTION__ << "(). entry point");

    // Генерация строковых представлений типов полей
    std::vector<std::string> fieldTypeStrings = generateFieldTypeStrings(scheme);

    // Корректируем типы данных в соответствии с размером строки
    adjustFieldTypeStringsForRowSize(fieldTypeStrings);

    std::stringstream query;
    query << "CREATE TABLE " << scheme.name << " (ID int AUTO_INCREMENT, ";

    for (size_t i = 0; i < scheme.fields.size(); ++i) {
        if (scheme.fields[i].get(FIELD_NAME).asString() != KEY_FIELD) {
            query << fieldTypeStrings[i] << ", ";
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

    std::string engine = "MyISAM";
    if (scheme.databaseEngines.find("MySQL") != scheme.databaseEngines.end()) {
        engine = scheme.databaseEngines.at("MySQL");
    }

    if (
        addUniqueConstraints(scheme.uniqueFieldsSets, scheme.fields, scheme.name) != 0 ||
        createIndexes(scheme.indexes, scheme.name) != 0 ||
        setTableEngine(scheme.name, engine) != 0 ||
        buildRelations(scheme.relations, scheme.name) != 0
        ) {
        return -1;
    }

    LOG(LDEBUG, __FUNCTION__ << "(). end point.");
    return 0;
}

std::vector<std::string> MySqlDatabaseController::generateFieldTypeStrings(const TableScheme& scheme) {
    std::vector<std::string> fieldTypeStrings;
    for (const auto& field : scheme.fields) {
        fieldTypeStrings.emplace_back(generateFieldDefinition(field));
    }
    return fieldTypeStrings;
}

std::string MySqlDatabaseController::generateFieldDefinition(const StructureField& field) {
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

void MySqlDatabaseController::adjustFieldTypeStringsForRowSize(std::vector<std::string>& fieldTypeStrings) {
    LOG(LDEBUG, __FUNCTION__ << "() entry point")
    size_t totalRowSize = calculateRowSizeFromStrings(fieldTypeStrings);
    if (totalRowSize > MAX_ROW_SIZE) {
        LOG(LDEBUG, __FUNCTION__ << "() row size more than limit. Types will be optimized.")
        for (auto& fieldTypeString : fieldTypeStrings) {
            size_t pos = fieldTypeString.find("varbinary(");
            if (pos != std::string::npos) {
                int length;
                if (sscanf(fieldTypeString.c_str() + pos, "varbinary(%d)", &length) == 1) {
                    fieldTypeString.replace(pos, fieldTypeString.find(')', pos) - pos + 1, "blob(" + std::to_string(length) +")");
                }
                continue;
            }

            pos = fieldTypeString.find("varchar(");
            if (pos != std::string::npos) {
                int length;
                if (sscanf(fieldTypeString.c_str() + pos, "varchar(%d)", &length) == 1) {
                    fieldTypeString.replace(pos, fieldTypeString.find(')', pos) - pos + 1, "text(" + std::to_string(length) +")");
                }
            }
        }

        totalRowSize = calculateRowSizeFromStrings(fieldTypeStrings);

        if (totalRowSize > MAX_ROW_SIZE) {
            THROW(SystemException, "Row size exceeds the maximum allowed size even after adjustments. Cannot create table.");
        }
        LOG(LDEBUG, __FUNCTION__ << "() types were optimized.")
    }
    LOG(LDEBUG, __FUNCTION__ << "() end point")
}

size_t MySqlDatabaseController::calculateRowSizeFromStrings(const std::vector<std::string>& fieldTypeStrings) {
    LOG(LDEBUG, __FUNCTION__ << "() entry point")
    size_t totalRowSize = 0;
    std::regex typeRegex(R"((boolean|varbinary|varchar|char|binary|tinyint|smallint|mediumint|int|bigint|float|double|decimal|date|datetime|timestamp|time|year|enum|set|blob|text|tinyblob|tinytext|mediumblob|mediumtext|longblob|longtext)\(?(\d*)\)?)");

    for (const auto& fieldTypeString : fieldTypeStrings) {
        LOG(LTRACE, __FUNCTION__ << "() parsing: " << fieldTypeString)
        std::smatch match;
        if (std::regex_search(fieldTypeString, match, typeRegex)) {
            std::string type = match[1].str();
            std::string lengthStr = match[2].str();
            int length = lengthStr.empty() ? 0 : std::stoi(lengthStr);

            LOG(LTRACE, __FUNCTION__ << "() parsed type: " << type << " parsed len: " << length)
            totalRowSize += calculateFieldSize(type, length);
        }
    }
    LOG(LDEBUG, __FUNCTION__ << "() calculated row size: " << totalRowSize)
    return totalRowSize;
}

size_t MySqlDatabaseController::calculateFieldSize(const std::string& typeName, int length) {
    if (typeName == "boolean" || typeName == "enum" || typeName == "set" || typeName == "tinyint" || typeName == "year") {
        return 1;  // Для enum/set (может зависеть от количества вариантов)
    } else if (typeName == "smallint") {
        return 2;
    } else if (typeName == "date" || typeName == "time" || typeName == "mediumint") {
        return 3;
    } else if (typeName == "int" || typeName == "float") {
        return 4;
    } else if (typeName == "bigint" || typeName == "double" || typeName == "datetime" || typeName == "timestamp") {
        return 8;
    } else if (typeName == "varbinary" || typeName == "binary" || typeName == "char") {
        return length;  // Размер соответствует длине
    } else if (typeName == "varchar") {
        return length * 3;  // UTF-8 кодировка (3 байта на символ)
    } else if (typeName == "blob" || typeName == "longblob" || typeName == "text" || typeName == "mediumtext") {
        return 12;  // Ориентировочный размер указателя для хранения больших данных
    } else {
        return 0;     // Если тип неизвестен, вернем 0
    }
}

int MySqlDatabaseController::addUniqueConstraints(const std::unordered_map<std::string, std::vector<std::string>>& uniqueFieldsSets, const std::vector<StructureField>& fields, const std::string& tableName) {
    LOG(LDEBUG, __FUNCTION__ << "() called. Table name: " << tableName);
    auto poolClient = getSqlPoolClient();

    for (const auto& fieldsSet : uniqueFieldsSets) {
        if (fieldsSet.second.empty()) continue;

        std::stringstream uniqueQuery;
        uniqueQuery << "ALTER TABLE " << tableName <<
                    " ADD CONSTRAINT " << fieldsSet.first << " UNIQUE (" << fromKeyFieldsString(fieldsSet.second, fields) << ");";

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

std::string MySqlDatabaseController::fromKeyFieldsString(const std::vector<std::string>& fieldsSet, const std::vector<StructureField>& fields) {
    std::stringstream fieldsSetString;

    for (const auto& fieldName : fieldsSet) {
        auto it = std::find_if(fields.begin(), fields.end(), [&](const StructureField& field) {
            return field.get(FIELD_NAME).asString() == fieldName;
        });

        if (it != fields.end()) {
            fieldsSetString << fieldName;

            auto type = it->get(FIELD_TYPE).asString();
            if (type == STRING_JSON_TYPE || type == BINARY_JSON_TYPE) {
                int length = DEFAULT_LENGTH;
                if (it->get(LENGTH_PROPERTY).isInt()) {
                    length = it->get(LENGTH_PROPERTY).asInt();
                }
                fieldsSetString << "(" << length << ")";
            }
            fieldsSetString << ", ";
        }
    }

    std::string result = fieldsSetString.str();
    if (!result.empty()) {
        result.erase(result.size() - 2);  // Удаляем последнюю ", "
    }
    return result;
}

int MySqlDatabaseController::createIndexes(const std::unordered_map<std::string, std::vector<std::string>>& indexes, const std::string& tableName) {
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

int MySqlDatabaseController::buildRelations(const std::unordered_map<std::string, Relation>& relations, const std::string& tableName) {
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

int MySqlDatabaseController::setTableEngine(const std::string& tableName, const std::string& engineName) {
    LOG(LDEBUG, __FUNCTION__ << "() called. Table name: " << tableName);
    std::stringstream engineQuery;
    engineQuery << "ALTER TABLE " << tableName << " ENGINE=" << engineName << ";";

    auto poolClient = getSqlPoolClient();
    try {
        poolClient->get()->update(engineQuery.str());
    } catch (Exception* ex) {
        LOG(LERROR, __FUNCTION__ << "(). Changing table engine failed: " << ex->getText() << " query: " << engineQuery.str());
        delete ex;
        return -1;
    }

    return 0;
}

bool MySqlDatabaseController::tableExists(const std::string& tableName) {
    // TODO work with database name in future
    LOG(LTRACE, __FUNCTION__ << "(). entry point");
    std::string query = "SELECT EXISTS (SELECT 1 FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_SCHEMA = 'TC-DB-MAIN' AND TABLE_NAME = '" + tableName + "') AS `EXISTS`;";

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

int MySqlDatabaseController::dropTable(const TableScheme& scheme) {
    LOG(LDEBUG, __FUNCTION__ << "(). Entry point.")

    if (!tableExists(scheme.name)) {
        LOG(LDEBUG, __FUNCTION__ << "(). Table " << scheme.name << " does not exist");
        return 0;
    }

    std::stringstream query;
    query << "DROP TABLE IF EXISTS " << scheme.name << ";";

    auto poolClient = getSqlPoolClient();
    try {
        poolClient->get()->update(query.str());
        LOG(LDEBUG,__FUNCTION__ << "(). Table dropped successfully: " << scheme.name);
    } catch (Exception* ex) {
        LOG(LERROR,__FUNCTION__ << "(). Drop failed: " << ex->getText() << " query: " << query.str());
        delete ex;
        return -2;
    }

    LOG(LDEBUG, __FUNCTION__ << "(). End point.")
    return 0;
}

void MySqlDatabaseController::resetDatabases() {
    TRACE(__FUNCTION__ << "(). Entry point.")
    // TODO
    TRACE(__FUNCTION__ << "(). End point.")
}

void MySqlDatabaseController::createDatabase(const std::string& dbName) {
    TRACE(__FUNCTION__ << "(). Entry point.")
    // TODO
    TRACE(__FUNCTION__ << "(). End point.")
}

void MySqlDatabaseController::dropDatabase(const std::string& dbName) {
    TRACE(__FUNCTION__ << "(). Entry point.")
    // TODO
    TRACE(__FUNCTION__ << "(). End point.")
}

int MySqlDatabaseController::getVersionFromVersionTable(const std::string& tableName) {
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

int MySqlDatabaseController::getEntityVersion(const std::string& tableName) {
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

int MySqlDatabaseController::upsertTableVersion(const std::string& tableName, int version) {
    LOG(LDEBUG, __FUNCTION__ << "(). Entry point.");

    std::ostringstream query;
    query << "INSERT INTO " << StringTools::toLower(VERSIONS_TABLE_NAME)
          << " (TABLENAME, VERSION) VALUES ('"
          << tableName << "', " << version << ")"
          << " ON DUPLICATE KEY UPDATE VERSION = VALUES(VERSION);";

    auto poolClient = getSqlPoolClient();
    try {
        poolClient->get()->update(query.str());
    } catch (Exception* ex) {
        LOG(LERROR, __FUNCTION__ << "(). Insert/Update failed: " << ex->getText() << " for table: " << tableName);
        delete ex;
        return -1;
    }
    LOG(LDEBUG, __FUNCTION__ << "(). Success for table: " << tableName);
    return 0;
}
