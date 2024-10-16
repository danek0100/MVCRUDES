#include    <include/database/controllers/postgresql/PostgreSqlSchemeUpdater.h>

PostgreSqlSchemeUpdater::PostgreSqlSchemeUpdater(std::shared_ptr<ITypesConverter> typesConverter,
                                                 std::shared_ptr<IDatabaseController> databaseController):
                                                 typesConverter(std::move(typesConverter)),
                                                 databaseController(std::move(databaseController)) {}

PostgreSqlSchemeUpdater::~PostgreSqlSchemeUpdater() = default;

int PostgreSqlSchemeUpdater::updateCurrentScheme(const TableScheme& currentScheme, const TableScheme& newScheme) {
    TRACE(__FUNCTION__ << "(). Entry point.");
    std::vector<std::string> scripts;

    // Карта текущих полей для быстрого доступа
    std::unordered_map<std::string, StructureField> currentFieldsMap;
    for (const auto& field : currentScheme.fields) {
        currentFieldsMap[field.get(FIELD_NAME).asString()] = field;
    }

    std::unordered_map<std::string, StructureField> newFieldsMap;
    for (const auto& field : newScheme.fields) {
        newFieldsMap[field.get(FIELD_NAME).asString()] = field;
    }

    // Обработка новых и обновленных полей
    for (const auto& newField : newFieldsMap) {
        if (currentFieldsMap.find(newField.first) == currentFieldsMap.end()) {
            addNewFieldToTable(newScheme.name, newField.second, scripts);
        } else {
            const StructureField& currentField = currentFieldsMap[newField.first];
            updateFieldType(newScheme.name, currentField, newField.second, scripts);
            updateFieldDefault(newScheme.name, currentField, newField.second, scripts);
            updateFieldNullability(newScheme.name, currentField, newField.second, scripts);
            updateUniqueConstraint(newScheme.name, currentField, newField.second, scripts);
        }
    }

    // Удаление старых полей, которые не существуют в новой схеме
    for (const auto& currentField : currentFieldsMap) {
        if (newFieldsMap.find(currentField.first) == newFieldsMap.end()) {
            scripts.push_back("ALTER TABLE " + currentScheme.name + " DROP COLUMN " + currentField.first + ";");
        }
    }

    // Выполнение всех собранных скриптов
    // TODO рассмотреть возможность использования транзакций
    SqlPoolClient poolClient;
    for (const auto& script : scripts) {
        try {
            poolClient.get()->update(script);
        } catch (Exception* ex) {
            TRACE(__FUNCTION__ << "(). Script execution failed: " << ex->getText() << " query: " << script);
            delete ex;
            return -1;
        }
    }

    // Уникальность, индексы, (м.б. движки)

    return databaseController->upsertTableVersion(newScheme.name, newScheme.version);
}

void PostgreSqlSchemeUpdater::addNewFieldToTable(const std::string& tableName, const StructureField& field, std::vector<std::string>& scripts) {
    if (EntitiesStructureCache::getSingletonInstance()->isTypeEnum(field.get(FIELD_TYPE).asString())) {
        std::stringstream typeQuery;

        typeQuery<< "DROP TYPE IF EXISTS " << field.get(FIELD_TYPE).asString() << ";";
        scripts.emplace_back(typeQuery.str());
        typeQuery = std::stringstream();

        typeQuery << "CREATE TYPE " << field.get(FIELD_TYPE).asString() << " AS ENUM(";
        for (const std::string& enumValue : EntitiesStructureCache::getSingletonInstance()->getEnumValues(field.get(FIELD_TYPE).asString())) {
            typeQuery << "'" << enumValue << "',";
        }
        typeQuery.seekp(-1, std::stringstream::cur);
        typeQuery << ")";
        scripts.emplace_back(typeQuery.str() + ";");
    }

    std::stringstream query;
    query << "ALTER TABLE " << tableName <<
          " ADD COLUMN " << field.get(FIELD_NAME).asString() << " " <<
          typesConverter->getMappedType(field.get(FIELD_TYPE).asString(), !field.get(LENGTH_PROPERTY).isInt() ? -1 : field.get(LENGTH_PROPERTY).asInt());

    if (field.get(DEFAULT_VALUE_PROPERTY).isInt() || field.get(DEFAULT_VALUE_PROPERTY).isString() || field.get(DEFAULT_VALUE_PROPERTY).isBool()) {
        query << " DEFAULT " << typesConverter->adaptValueToType(field.get(FIELD_TYPE).asString(), field.get(DEFAULT_VALUE_PROPERTY));
    }
    if (field.get(FIELD_NAME).asString() != "ID") {
        if (!field.get(CAN_BE_EMPTY_PROPERTY).isBool() || !field.get(CAN_BE_EMPTY_PROPERTY).asBool()) {
            query << " NOT NULL";
        }
        // Считаем, что Postgres выдаст стандартизированный уникальный ключ колонке
/*        if (field.get(IS_UNIQUE_PROPERTY).isBool() && field.get(IS_UNIQUE_PROPERTY).asBool()) {
            query << " UNIQUE";
        }*/
    }
    scripts.emplace_back(query.str() + ";");

    if (field.get(FIELD_NAME).asString() == "ID") {
        scripts.push_back("CREATE SEQUENCE " + StringTools::toLower(tableName) + "_id_seq;");
        scripts.push_back("ALTER TABLE " + StringTools::toLower(tableName) + " ALTER COLUMN ID SET DEFAULT nextval('" + StringTools::toLower(tableName) + "_id_seq');");
        scripts.push_back("ALTER SEQUENCE " + StringTools::toLower(tableName) + "_id_seq OWNED BY " + StringTools::toLower(tableName) + ".id;");
        // устанавливаем значения ID для уже существующих записей в таблице
        scripts.push_back("UPDATE " + StringTools::toLower(tableName) + " SET ID=nextval('" + StringTools::toLower(tableName) + "_id_seq');");
        scripts.push_back("ALTER TABLE " + StringTools::toLower(tableName) + " ALTER COLUMN ID SET NOT NULL;");
    }
}

void PostgreSqlSchemeUpdater::updateFieldType(const std::string& tableName, const StructureField& oldField,
                                                   const StructureField& newField, std::vector<std::string>& scripts) {
    std::stringstream query;
    bool typeChanged = false;
    bool lengthChanged = false;

    if (newField.get(FIELD_TYPE).isString() && oldField.get(FIELD_TYPE).isString()) {
        std::string newFieldType = newField.get(FIELD_TYPE).asString();
        std::string oldFieldType = oldField.get(FIELD_TYPE).asString();
        typeChanged = newFieldType != oldFieldType;
    }

    int newFieldLength = newField.get(LENGTH_PROPERTY).isInt() ? newField.get(LENGTH_PROPERTY).asInt() : -1;
    int oldFieldLength = oldField.get(LENGTH_PROPERTY).isInt() ? oldField.get(LENGTH_PROPERTY).asInt() : -1;
    lengthChanged = newFieldLength != oldFieldLength;

    if (typeChanged || lengthChanged) {
        std::string columnName = newField.get(FIELD_NAME).asString();
        std::string newType = typesConverter->getMappedType(newField.get(FIELD_TYPE).asString(), newFieldLength);

        std::stringstream newTypeQuery;
        newTypeQuery << "ALTER TABLE " << tableName << " ALTER COLUMN " << columnName << " TYPE " << newType;

        if (EntitiesStructureCache::getSingletonInstance()->isTypeEnum(newType)) {
            newTypeQuery << " USING " << columnName << "::" << newType;
        }

        scripts.emplace_back(newTypeQuery.str() + ";");
    }
}

void PostgreSqlSchemeUpdater::updateFieldDefault(const std::string& tableName, const StructureField& oldField,
                                                      const StructureField& newField, std::vector<std::string>& scripts) {
    bool defaultValueChanged = (newField.get(DEFAULT_VALUE_PROPERTY).isInt() || newField.get(DEFAULT_VALUE_PROPERTY).isString()) &&
                               (newField.get(DEFAULT_VALUE_PROPERTY) != oldField.get(DEFAULT_VALUE_PROPERTY));
    bool defaultValueRemoved = newField.get(DEFAULT_VALUE_PROPERTY).isNull() && !oldField.get(DEFAULT_VALUE_PROPERTY).isNull();

    if (defaultValueChanged) {
        scripts.emplace_back("ALTER TABLE " + tableName +
                             " ALTER COLUMN " + newField.get(FIELD_NAME).asString() +
                             " SET DEFAULT " + typesConverter->adaptValueToType(newField.get(FIELD_TYPE).asString(), newField.get(DEFAULT_VALUE_PROPERTY)) + ";");
    } else if (defaultValueRemoved) {
        scripts.emplace_back("ALTER TABLE " + tableName +
                             " ALTER COLUMN " + newField.get(FIELD_NAME).asString() +
                             " DROP DEFAULT;");
    }
}

void PostgreSqlSchemeUpdater::updateFieldNullability(const std::string& tableName, const StructureField& oldField,
                                                          const StructureField& newField, std::vector<std::string>& scripts) {
    bool isNewFieldNullable = !newField.get(CAN_BE_EMPTY_PROPERTY).isBool() || (newField.get(CAN_BE_EMPTY_PROPERTY).isBool() && newField.get(CAN_BE_EMPTY_PROPERTY).asBool());
    bool isOldFieldNullable = !oldField.get(CAN_BE_EMPTY_PROPERTY).isBool() || (oldField.get(CAN_BE_EMPTY_PROPERTY).isBool() && oldField.get(CAN_BE_EMPTY_PROPERTY).asBool());

    if (!isNewFieldNullable && isOldFieldNullable) {
        scripts.emplace_back("ALTER TABLE " + tableName +
                             " ALTER COLUMN " + newField.get(FIELD_NAME).asString() +
                             " SET NOT NULL;");
    }

    if (isNewFieldNullable && !isOldFieldNullable) {
        scripts.emplace_back("ALTER TABLE " + tableName +
                             " ALTER COLUMN " + newField.get(FIELD_NAME).asString() +
                             " DROP NOT NULL;");
    }
}


void PostgreSqlSchemeUpdater::updateUniqueConstraint(const std::string& tableName, const StructureField& oldField,
                                                          const StructureField& newField, std::vector<std::string>& scripts) {
/*
    bool uniqueRequired = newField.get(IS_UNIQUE_PROPERTY).isBool() && newField.get(IS_UNIQUE_PROPERTY).asBool();
    bool uniqueExists = oldField.get(IS_UNIQUE_PROPERTY).isBool() && oldField.get(IS_UNIQUE_PROPERTY).asBool();

    std::string uniqueConstraintName = StringTools::toLower(tableName) + "_" + StringTools::toLower(newField.get(FIELD_NAME).asString()) + "_key";

    if (uniqueRequired && !uniqueExists) {
        scripts.emplace_back("ALTER TABLE " + tableName +
                          " ADD CONSTRAINT " + uniqueConstraintName + " UNIQUE (" + newField.get(FIELD_NAME).asString() + ");");
    } else if (!uniqueRequired && uniqueExists) {
        scripts.emplace_back("ALTER TABLE " + tableName +
                          " DROP CONSTRAINT IF EXISTS " + uniqueConstraintName + ";");
    }
*/
}