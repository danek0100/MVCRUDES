#include    <include/database/controllers/postgresql/PostgreSqlSchemeLoader.h>

PostgreSqlSchemeLoader::PostgreSqlSchemeLoader(std::shared_ptr<ITypesConverter> typesConverter):
                                               typesConverter(std::move(typesConverter)),
                                               structures(EntitiesStructureCache::getSingletonInstance()) {}

PostgreSqlSchemeLoader::PostgreSqlSchemeLoader(std::shared_ptr<ITypesConverter> typesConverter,
                                               std::shared_ptr<IEntitiesStructureCache> structures):
                                               typesConverter(std::move(typesConverter)),
                                               structures(std::move(structures)) {}

PostgreSqlSchemeLoader::~PostgreSqlSchemeLoader() = default;

TableScheme PostgreSqlSchemeLoader::getActualScheme(const std::string& tableName) {
    LOG(LDEBUG, __FUNCTION__ << "(). Entry point.");

    auto fields = loadFields(tableName);
    auto indexes = loadIndexes(tableName);
    auto uniqueSets = loadUniqueSets(tableName);
    auto relations = loadRelations(tableName);

    LOG(LDEBUG, __FUNCTION__ << "(). End point.");
    return {tableName, -1, true, fields, indexes, uniqueSets, relations, {}};
}

std::vector<StructureField> PostgreSqlSchemeLoader::loadFields(const std::string& tableName) {
    LOG(LTRACE, __FUNCTION__ << "() entry point");
    std::vector<StructureField> fields = {};
    std::string table = StringTools::toLower(tableName);

    std::string query = R"(
    SELECT
        c.column_name::text,
        CASE
            WHEN c.data_type = 'information_schema.sql_identifier' THEN 'varchar'
            WHEN c.data_type = 'information_schema.character_data' THEN 'text'
            WHEN c.data_type = 'information_schema.yes_or_no' THEN 'boolean'
            WHEN c.data_type = 'information_schema.cardinal_number' THEN 'integer'
            WHEN c.data_type = 'USER-DEFINED' then UPPER(c.udt_name)
            ELSE c.data_type::text
        END AS data_type,
        c.is_nullable::boolean,
        CASE
            WHEN c.column_default LIKE 'nextval(%'::text THEN 'auto_increment'
            WHEN c.column_default LIKE 'NULL::%'::text THEN 'NULL'
            ELSE c.column_default::text
        END AS column_default,
        c.character_maximum_length::integer
    FROM information_schema.columns AS c
    WHERE c.table_name = ')";

    query += table + R"('
    ORDER BY c.ordinal_position;
    )"; // TODO work with Database in future

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

StructureField PostgreSqlSchemeLoader::getStructureFieldFormSqlResult(ISqlResult& sqlResult) {
    StructureField field;
    field.put(FIELD_NAME, {StringTools::toUpper(sqlResult.getString("COLUMN_NAME"))});
    field.put(FIELD_TYPE, {typesConverter->getUnmappedType(sqlResult.getString("DATA_TYPE"))});
    field.put(CAN_BE_EMPTY_PROPERTY, {sqlResult.getInt("IS_NULLABLE") == 1});
    if (field.get(FIELD_TYPE) == INT_JSON_TYPE && sqlResult.isDataAvailable("COLUMN_DEFAULT")) {
        std::string default_value = sqlResult.getString("COLUMN_DEFAULT");
        if (default_value != "auto_increment") {
            try {
                int value = std::stoi(default_value);
                field.put(DEFAULT_VALUE_PROPERTY, {value});
            } catch (const std::invalid_argument& e) {
                LOG(LWARN, __FUNCTION__ << "(). '" << default_value << "' cannot be parsed. " << e.what());
            } catch (const std::out_of_range& e) {
                LOG(LWARN, __FUNCTION__ << "(). '" << default_value << "' cannot be parsed. " << e.what());
            }
        }
    } else if (field.get(FIELD_TYPE) == FLOAT_JSON_TYPE && sqlResult.isDataAvailable("COLUMN_DEFAULT")) {
        std::string default_value = sqlResult.getString("COLUMN_DEFAULT");
        try {
            double value = std::stod(default_value);
            field.put(DEFAULT_VALUE_PROPERTY, {value});
        } catch (const std::invalid_argument& e) {
            LOG(LWARN, __FUNCTION__ << "(). '" << default_value << "' cannot be parsed. " << e.what());
        } catch (const std::out_of_range& e) {
            LOG(LWARN, __FUNCTION__ << "(). '" << default_value << "' cannot be parsed. " << e.what());
        }
    } else if (field.get(FIELD_TYPE) == STRING_JSON_TYPE) {
        field.put(LENGTH_PROPERTY, {sqlResult.getInt("CHARACTER_MAXIMUM_LENGTH")});
        if (sqlResult.isDataAvailable("COLUMN_DEFAULT")) {
            std::string default_value = sqlResult.getString("COLUMN_DEFAULT");
            size_t start_pos = default_value.find('\'');
            size_t end_pos = default_value.rfind('\'');
            if (start_pos != std::string::npos && end_pos != std::string::npos && start_pos != end_pos) {
                default_value = default_value.substr(start_pos + 1, end_pos - start_pos - 1);
            }
            field.put(DEFAULT_VALUE_PROPERTY, {default_value});
        }
    } else if (field.get(FIELD_TYPE) == BOOLEAN_JSON_TYPE && sqlResult.isDataAvailable("COLUMN_DEFAULT")) {
        std::string default_value = sqlResult.getString("COLUMN_DEFAULT");
        try {
            bool value = std::stoi(default_value) == 1;
            field.put(DEFAULT_VALUE_PROPERTY, {value});
        } catch (const std::invalid_argument& e) {
            LOG(LWARN, __FUNCTION__ << "(). '" << default_value << "' cannot be parsed. " << e.what());
        } catch (const std::out_of_range& e) {
            LOG(LWARN, __FUNCTION__ << "(). '" << default_value << "' cannot be parsed. " << e.what());
        }
    } else if (field.get(FIELD_TYPE) == BINARY_JSON_TYPE && sqlResult.isDataAvailable("COLUMN_DEFAULT")) {
        std::string default_value = sqlResult.getRaw("COLUMN_DEFAULT").toRawHex();
        field.put(DEFAULT_VALUE_PROPERTY, {default_value});
    } else if (field.get(FIELD_TYPE) == DATE_JSON_TYPE && sqlResult.isDataAvailable("COLUMN_DEFAULT")) {
        std::string default_value = sqlResult.getString("COLUMN_DEFAULT");
        if (default_value != "now()") {
            field.put(DEFAULT_VALUE_PROPERTY, {sqlResult.getDate("COLUMN_DEFAULT").get()});
        }
    } else if (field.get(FIELD_TYPE) == DATETIME_JSON_TYPE && sqlResult.isDataAvailable("COLUMN_DEFAULT")) {
        std::string default_value = sqlResult.getString("COLUMN_DEFAULT");
        if (default_value != "now()" && default_value != "NULL::timestamp without time zone") {
            field.put(DEFAULT_VALUE_PROPERTY, {sqlResult.getDatetime("COLUMN_DEFAULT").get()});
        }
    } else if (structures->isTypeEnum(field.get(FIELD_TYPE).asString()) && sqlResult.isDataAvailable("COLUMN_DEFAULT")) {
        std::string default_value = sqlResult.getString("COLUMN_DEFAULT");
        size_t start_pos = default_value.find('\'');
        size_t end_pos = default_value.rfind('\'');
        if (start_pos != std::string::npos && end_pos != std::string::npos && start_pos != end_pos) {
            default_value = default_value.substr(start_pos + 1, end_pos - start_pos - 1);
        }
        field.put(DEFAULT_VALUE_PROPERTY, {default_value});
    }
    LOG(LTRACE, __FUNCTION__ << "() field: " << field);
    return field;
}

std::unordered_map<std::string, std::vector<std::string>> PostgreSqlSchemeLoader::loadIndexes(const std::string& tableName) {
    LOG(LTRACE, __FUNCTION__ << "() entry point");
    std::unordered_map<std::string, std::vector<std::string>> indexes;
    std::string table = StringTools::toLower(tableName);

    std::string query = R"(
        SELECT
            i.relname AS index_name,
            a.attname AS column_name
        FROM
            pg_class t,
            pg_class i,
            pg_index ix,
            pg_attribute a,
            pg_namespace ns
        WHERE
            t.oid = ix.indrelid
            AND i.oid = ix.indexrelid
            AND a.attrelid = t.oid
            AND a.attnum = ANY(ix.indkey)
            AND t.relkind = 'r'
            AND t.relname = ')" + table + R"('
            AND ns.oid = t.relnamespace
            AND ns.nspname = 'public'
        ORDER BY
            i.relname, a.attnum;
    )";

    auto poolClient = getSqlPoolClient();
    try {
        auto sqlResult = poolClient->get()->query(query);

        std::string currentIndexName;
        std::vector<std::string> currentIndexColumns;

        while (sqlResult->next()) {
            std::string indexName = sqlResult->getString("index_name");
            std::string columnName = sqlResult->getString("column_name");

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

std::unordered_map<std::string, std::vector<std::string>> PostgreSqlSchemeLoader::loadUniqueSets(const std::string& tableName) {
    LOG(LTRACE, __FUNCTION__ << "() entry point");
    std::unordered_map<std::string, std::vector<std::string>> sets;
    std::string table = StringTools::toLower(tableName);

    std::string query = R"(
        SELECT
            i.relname AS index_name,
            a.attname AS column_name
        FROM
            pg_class t,
            pg_class i,
            pg_index ix,
            pg_attribute a,
            pg_namespace ns
        WHERE
            t.oid = ix.indrelid
            AND i.oid = ix.indexrelid
            AND a.attrelid = t.oid
            AND a.attnum = ANY(ix.indkey)
            AND t.relkind = 'r'
            AND t.relname = ')" + table + R"('
            AND ix.indisunique = true
            AND ns.oid = t.relnamespace
            AND ns.nspname = 'public'
        ORDER BY
            i.relname, a.attnum;
    )";

    auto poolClient = getSqlPoolClient();
    try {
        auto sqlResult = poolClient->get()->query(query);

        std::string currentSetName;
        std::vector<std::string> currentSetColumns;

        while (sqlResult->next()) {
            std::string uniqueSetName = sqlResult->getString("index_name");
            std::string columnName = sqlResult->getString("column_name");

            if (uniqueSetName.rfind("_ukey") != (uniqueSetName.size() - 5)) {
                continue; // не уникальный ключ
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

std::unordered_map<std::string, Relation> PostgreSqlSchemeLoader::loadRelations(const std::string& tableName) {
    LOG(LTRACE, __FUNCTION__ << "() entry point");
    std::unordered_map<std::string, Relation> relations;
    std::string table = StringTools::toLower(tableName);

    std::string query = R"(
        SELECT
            con.conname AS constraint_name,
            att2.attname AS local_column,
            cl.relname AS referenced_table,
            att.attname AS referenced_column,
            con.confupdtype AS on_update,
            con.confdeltype AS on_delete
        FROM
            pg_constraint con
        JOIN
            pg_class cl ON con.confrelid = cl.oid
        JOIN
            pg_attribute att ON att.attnum = ANY(con.confkey) AND att.attrelid = cl.oid
        JOIN
            pg_class tbl ON tbl.oid = con.conrelid
        JOIN
            pg_attribute att2 ON att2.attnum = ANY(con.conkey) AND att2.attrelid = tbl.oid
        WHERE
            con.contype = 'f'
            AND tbl.relname = ')" + table + R"('
        ORDER BY
            con.conname, att2.attnum;
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
            std::string constraintName = sqlResult->getString("constraint_name");

            if (constraintName.rfind("_fkey") != (constraintName.size() - 5)) {
                continue; // не внешний ключ
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
            onDelete = Relation::strToRelationType(getPostgresAction(sqlResult->getString("on_delete")));
            onUpdate = Relation::strToRelationType(getPostgresAction(sqlResult->getString("on_update")));
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

std::string PostgreSqlSchemeLoader::getPostgresAction(const std::string& action) {
    if (action == "a") return "NO ACTION";
    if (action == "r") return "RESTRICT";
    if (action == "c") return "CASCADE";
    if (action == "n") return "SET NULL";
    if (action == "d") return "SET DEFAULT";
    return "UNKNOWN";
}
