#include    <include/database/controllers/mysql/MySqlSchemeLoader.h>

MySqlSchemeLoader::MySqlSchemeLoader(std::shared_ptr<ITypesConverter> typesConverter):
                                     typesConverter(std::move(typesConverter)),
                                     structures(EntitiesStructureCache::getSingletonInstance()) {}

MySqlSchemeLoader::MySqlSchemeLoader(std::shared_ptr<ITypesConverter> typesConverter,
                                     std::shared_ptr<IEntitiesStructureCache> structures):
                                     typesConverter(std::move(typesConverter)),
                                     structures(std::move(structures)) {}

MySqlSchemeLoader::~MySqlSchemeLoader() = default;

TableScheme MySqlSchemeLoader::getActualScheme(const std::string& tableName) {
    LOG(LDEBUG, __FUNCTION__ << "() entry point");

    auto fields = loadFields(tableName);
    auto indexes = loadIndexes(tableName);
    auto uniqueSets = loadUniqueSets(tableName);
    auto relations = loadRelations(tableName);
    auto engine = loadTableEngine(tableName);

    LOG(LDEBUG, __FUNCTION__ << "() end point");
    return {tableName, -1, true, fields, indexes, uniqueSets, relations, engine};
}

std::vector<StructureField> MySqlSchemeLoader::loadFields(const std::string& tableName) {
    LOG(LTRACE, __FUNCTION__ << "() entry point");
    std::vector<StructureField> fields = {};
    std::string query = R"(
    SELECT
        c.COLUMN_NAME AS `column_name`,
        CASE
            WHEN c.DATA_TYPE = 'enum' THEN CONCAT('enum(', UPPER(c.TABLE_NAME), '_', c.COLUMN_NAME, ',', c.COLUMN_TYPE, ')')
            WHEN c.DATA_TYPE = 'varchar' THEN 'varchar'
            WHEN c.DATA_TYPE = 'text' THEN 'text'
            WHEN c.DATA_TYPE = 'tinyint' AND c.COLUMN_TYPE = 'tinyint(1)' THEN 'boolean'
            WHEN c.DATA_TYPE IN ('bigint', 'mediumint', 'smallint', 'tinyint', 'int') THEN 'integer'
            ELSE c.DATA_TYPE
        END AS `data_type`,
        c.IS_NULLABLE = 'YES' AS `is_nullable`,
        CASE
            WHEN c.EXTRA = 'auto_increment' THEN 'auto_increment'
            ELSE c.COLUMN_DEFAULT
        END AS `column_default`,
        c.CHARACTER_MAXIMUM_LENGTH AS `character_maximum_length`
    FROM INFORMATION_SCHEMA.COLUMNS AS c
    WHERE c.TABLE_NAME = ')";

    query += tableName + R"('
        AND c.TABLE_SCHEMA = 'TC-DB-MAIN'
    ORDER BY c.ORDINAL_POSITION;)"; // TODO work with Database in future

    auto poolClient = getSqlPoolClient();
    try {
        auto sqlResult = poolClient->get()->query(query);
        while (sqlResult->next()) {
            fields.emplace_back(getStructureFieldFormSqlResult(*sqlResult));
        }
    } catch (Exception* ex) {
        LOG(LERROR, __FUNCTION__ << "(). Parsing structure failed: " << ex->getText() << " query: " << query);
        delete ex;
        THROW(SystemException, "Scheme for " << tableName << " cannot be loaded!")
    }
    LOG(LTRACE, __FUNCTION__ << "() end point");
    return fields;
}

StructureField MySqlSchemeLoader::getStructureFieldFormSqlResult(ISqlResult& sqlResult) {
    StructureField field;
    field.put(FIELD_NAME, {StringTools::toUpper(sqlResult.getString("column_name"))});
    field.put(FIELD_TYPE, {typesConverter->getUnmappedType(sqlResult.getString("data_type"))});
    field.put(CAN_BE_EMPTY_PROPERTY, {sqlResult.getInt("is_nullable") == 1});
    if (field.get(FIELD_TYPE) == INT_JSON_TYPE && sqlResult.isDataAvailable("column_default")) {
        std::string default_value = sqlResult.getString("column_default");
        if (default_value != "auto_increment" && default_value != "NULL") {
            try {
                int value = std::stoi(default_value);
                field.put(DEFAULT_VALUE_PROPERTY, {value});
            } catch (const std::invalid_argument& e) {
                LOG(LWARN, __FUNCTION__ << "(). '" << default_value << "' cannot be parsed. " << e.what());
            } catch (const std::out_of_range& e) {
                LOG(LWARN, __FUNCTION__ << "(). '" << default_value << "' cannot be parsed. " << e.what());
            }
        }
    } else if (field.get(FIELD_TYPE) == FLOAT_JSON_TYPE && sqlResult.isDataAvailable("column_default")) {
        std::string default_value = sqlResult.getString("column_default");
        if (default_value != "NULL")
            try {
                double value = std::stod(default_value);
                field.put(DEFAULT_VALUE_PROPERTY, {value});
            } catch (const std::invalid_argument& e) {
                LOG(LWARN, __FUNCTION__ << "(). '" << default_value << "' cannot be parsed. " << e.what());
            } catch (const std::out_of_range& e) {
                LOG(LWARN, __FUNCTION__ << "(). '" << default_value << "' cannot be parsed. " << e.what());
            }
    } else if (field.get(FIELD_TYPE) == STRING_JSON_TYPE) {
        field.put(LENGTH_PROPERTY, {sqlResult.getInt("character_maximum_length")});
        if (sqlResult.isDataAvailable("column_default")) {
            std::string default_value = sqlResult.getString("column_default");
            if (default_value != "NULL") {
                size_t start_pos = default_value.find('\'');
                size_t end_pos = default_value.rfind('\'');
                if (start_pos != std::string::npos && end_pos != std::string::npos && start_pos != end_pos) {
                    default_value = default_value.substr(start_pos + 1, end_pos - start_pos - 1);
                }
                field.put(DEFAULT_VALUE_PROPERTY, {default_value});
            }
        }
    } else if (field.get(FIELD_TYPE) == BOOLEAN_JSON_TYPE && sqlResult.isDataAvailable("column_default")) {
        std::string default_value = sqlResult.getString("column_default");
        if (default_value != "NULL") {
            try {
                bool value = std::stoi(default_value) == 1;
                field.put(DEFAULT_VALUE_PROPERTY, {value});
            } catch (const std::invalid_argument &e) {
                LOG(LWARN, __FUNCTION__ << "(). '" << default_value << "' cannot be parsed. " << e.what());
            } catch (const std::out_of_range &e) {
                LOG(LWARN, __FUNCTION__ << "(). '" << default_value << "' cannot be parsed. " << e.what());
            }
        }
    } else if (field.get(FIELD_TYPE) == BINARY_JSON_TYPE) {
        field.put(LENGTH_PROPERTY, {sqlResult.getInt("character_maximum_length")});
        if (sqlResult.isDataAvailable("column_default") && sqlResult.getString("column_default") != "NULL") {
            std::string default_value = sqlResult.getRaw("column_default").toRawHex();
            field.put(DEFAULT_VALUE_PROPERTY, {default_value});
        }
    } else if (field.get(FIELD_TYPE) == DATE_JSON_TYPE && sqlResult.isDataAvailable("column_default")) {
        std::string default_value = sqlResult.getString("column_default");
        if (default_value != "NULL" && default_value != "current_timestamp()") {
            field.put(DEFAULT_VALUE_PROPERTY, {sqlResult.getDate("column_default").get()});
        }
    } else if (field.get(FIELD_TYPE) == DATETIME_JSON_TYPE && sqlResult.isDataAvailable("column_default")) {
        std::string default_value = sqlResult.getString("column_default");
        if (default_value != "NULL" && default_value != "current_timestamp()") {
            field.put(DEFAULT_VALUE_PROPERTY, {sqlResult.getDatetime("column_default").get()});
        }
    } else if (structures->isTypeEnum(field.get(FIELD_TYPE).asString()) && sqlResult.isDataAvailable("column_default")) {
        std::string default_value = sqlResult.getString("column_default");
        if (sqlResult.getString("column_default") != "NULL") {
            size_t start_pos = default_value.find('\'');
            size_t end_pos = default_value.rfind('\'');
            if (start_pos != std::string::npos && end_pos != std::string::npos && start_pos != end_pos) {
                default_value = default_value.substr(start_pos + 1, end_pos - start_pos - 1);
            }
            field.put(DEFAULT_VALUE_PROPERTY, {default_value});
        }
    }
    LOG(LTRACE, __FUNCTION__ << "() field: " << field)
    return field;
}

std::unordered_map<std::string, std::vector<std::string>> MySqlSchemeLoader::loadIndexes(const std::string& tableName) {
    LOG(LTRACE, __FUNCTION__ << "() entry point");
    std::unordered_map<std::string, std::vector<std::string>> indexes;

    std::string query = R"(
        SELECT
            INDEX_NAME,
            COLUMN_NAME
        FROM
            INFORMATION_SCHEMA.STATISTICS
        WHERE
            TABLE_NAME = ')" + tableName + R"('
            AND TABLE_SCHEMA = 'TC-DB-MAIN'
        ORDER BY
            INDEX_NAME, SEQ_IN_INDEX;
    )";

    auto poolClient = getSqlPoolClient();
    try {
        auto sqlResult = poolClient->get()->query(query);

        std::string currentIndexName;
        std::vector<std::string> currentIndexColumns;

        while (sqlResult->next()) {
            std::string indexName = sqlResult->getString("INDEX_NAME");
            std::string columnName = sqlResult->getString("COLUMN_NAME");

            if (indexName.rfind("_idx") != (indexName.size() - 4)) {
                continue; // не индекс
            }

            if (indexName != currentIndexName && !currentIndexColumns.empty()) {
                indexes[currentIndexName] = currentIndexColumns;
                currentIndexColumns.clear();
            }

            currentIndexName = indexName;
            currentIndexColumns.push_back(columnName);
        }

        if (!currentIndexColumns.empty()) {
            indexes[currentIndexName] = currentIndexColumns;
        }
    } catch (Exception* ex) {
        LOG(LERROR, __FUNCTION__ << "(). Parsing indexes failed: " << ex->getText() << " query: " << query);
        delete ex;
        THROW(SystemException, "Indexes for " << tableName << " cannot be loaded!");
    }
    LOG(LTRACE, __FUNCTION__ << "() end point");
    return indexes;
}

std::unordered_map<std::string, std::vector<std::string>> MySqlSchemeLoader::loadUniqueSets(const std::string& tableName) {
    LOG(LTRACE, __FUNCTION__ << "() entry point");
    std::unordered_map<std::string, std::vector<std::string>> sets;

    std::string query = R"(
        SELECT
            INDEX_NAME,
            COLUMN_NAME
        FROM
            INFORMATION_SCHEMA.STATISTICS
        WHERE
            TABLE_NAME = ')" + tableName + R"('
            AND TABLE_SCHEMA = 'TC-DB-MAIN'
            AND NON_UNIQUE = 0
        ORDER BY
            INDEX_NAME, SEQ_IN_INDEX;
    )";

    auto poolClient = getSqlPoolClient();
    try {
        auto sqlResult = poolClient->get()->query(query);

        std::string currentSetName;
        std::vector<std::string> currentSetColumns;

        while (sqlResult->next()) {
            std::string uniqueSetName = sqlResult->getString("INDEX_NAME");
            std::string columnName = sqlResult->getString("COLUMN_NAME");

            if (uniqueSetName.rfind("_ukey") != (uniqueSetName.size() - 5)) {
                continue; // не индекс
            }

            if (uniqueSetName != currentSetName && !currentSetColumns.empty()) {
                sets[currentSetName] = currentSetColumns;
                currentSetColumns.clear();
            }

            currentSetName = uniqueSetName;
            currentSetColumns.push_back(columnName);
        }

        if (!currentSetColumns.empty()) {
            sets[currentSetName] = currentSetColumns;
        }
    } catch (Exception* ex) {
        LOG(LERROR, __FUNCTION__ << "(). Parsing unique sets failed: " << ex->getText() << " query: " << query);
        delete ex;
        THROW(SystemException, "Unique sets for " << tableName << " cannot be loaded!");
    }
    LOG(LTRACE, __FUNCTION__ << "() end point");
    return sets;
}

std::unordered_map<std::string, Relation> MySqlSchemeLoader::loadRelations(const std::string& tableName) {
    LOG(LTRACE, __FUNCTION__ << "() entry point");
    std::unordered_map<std::string, Relation> relations;

    std::string query = R"(
        SELECT
            kcu.CONSTRAINT_NAME,
            kcu.COLUMN_NAME AS local_column,
            kcu.REFERENCED_TABLE_NAME AS referenced_table,
            kcu.REFERENCED_COLUMN_NAME AS referenced_column,
            rc.UPDATE_RULE AS on_update,
            rc.DELETE_RULE AS on_delete
        FROM
            INFORMATION_SCHEMA.KEY_COLUMN_USAGE AS kcu
        JOIN
            INFORMATION_SCHEMA.REFERENTIAL_CONSTRAINTS AS rc
        ON
            kcu.CONSTRAINT_NAME = rc.CONSTRAINT_NAME
        WHERE
            kcu.TABLE_NAME = ')" + tableName + R"('
            AND kcu.TABLE_SCHEMA = 'TC-DB-MAIN'
            AND kcu.REFERENCED_TABLE_NAME IS NOT NULL
        ORDER BY
            kcu.CONSTRAINT_NAME, kcu.ORDINAL_POSITION;
    )";

    auto poolClient = getSqlPoolClient();
    try {
        auto sqlResult = poolClient->get()->query(query);

        std::string currentConstraintName;
        std::vector<std::string> localColumns;
        std::vector<std::string> referencedColumns;
        std::string referencedTable;
        Relation::RelationType onDelete;
        Relation::RelationType onUpdate;

        while (sqlResult->next()) {
            std::string constraintName = sqlResult->getString("CONSTRAINT_NAME");

            if (constraintName.rfind("_fkey") != (constraintName.size() - 5)) {
                continue; // не наш внешний ключ
            }

            if (constraintName != currentConstraintName && !localColumns.empty()) {
                relations[currentConstraintName] = {localColumns, referencedTable, referencedColumns, onDelete, onUpdate};
                localColumns.clear();
                referencedColumns.clear();
            }

            currentConstraintName = constraintName;
            localColumns.push_back(sqlResult->getString("local_column"));
            referencedTable = sqlResult->getString("referenced_table");
            referencedColumns.push_back(sqlResult->getString("referenced_column"));
            onDelete = Relation::strToRelationType(sqlResult->getString("on_delete"));
            onUpdate = Relation::strToRelationType(sqlResult->getString("on_update"));
        }

        if (!localColumns.empty()) {
            relations[currentConstraintName] = {localColumns, referencedTable, referencedColumns, onDelete, onUpdate};
        }
    } catch (Exception* ex) {
        LOG(LERROR, __FUNCTION__ << "(). Parsing foreign keys failed: " << ex->getText() << " query: " << query);
        delete ex;
        THROW(SystemException, "Foreign keys for " << tableName << " cannot be loaded!");
    }
    LOG(LTRACE, __FUNCTION__ << "() end point");
    return relations;
}

std::unordered_map<std::string, std::string> MySqlSchemeLoader::loadTableEngine(const std::string& tableName) {
    LOG(LTRACE, __FUNCTION__ << "() entry point");
    std::unordered_map<std::string, std::string> result;
    std::string query = R"(
        SELECT
            ENGINE
        FROM
            INFORMATION_SCHEMA.TABLES
        WHERE
            TABLE_NAME = ')" + tableName + R"('
            AND TABLE_SCHEMA = 'TC-DB-MAIN';
    )";

    auto poolClient = getSqlPoolClient();
    try {
        auto sqlResult = poolClient->get()->query(query);

        if (sqlResult->next()) {
            std::string engine = sqlResult->getString("ENGINE");
            result["MySQL"] = engine;
        } else {
            LOG(LWARN, __FUNCTION__ << "() Table '" + tableName + "' not found in schema 'TC-DB-MAIN'.");
        }
    } catch (Exception* ex) {
        LOG(LERROR, __FUNCTION__ << "(). Failed to get table engine: " << ex->getText() << " query: " << query);
        delete ex;
        THROW(SystemException, "Cannot load engine for table " << tableName << "!");
    }
    LOG(LTRACE, __FUNCTION__ << "() end point");
    return result;
}
