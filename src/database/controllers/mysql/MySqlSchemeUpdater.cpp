#include    <include/database/controllers/mysql/MySqlSchemeUpdater.h>
#include    <include/database/controllers/mysql/MySqlDatabaseController.h>

MySqlSchemeUpdater::MySqlSchemeUpdater(std::shared_ptr<ITypesConverter> typesConverter,
                                       std::shared_ptr<IDatabaseController> databaseController):
                                       typesConverter(std::move(typesConverter)),
                                       databaseController(std::move(databaseController)) {}

MySqlSchemeUpdater::~MySqlSchemeUpdater() = default;


int MySqlSchemeUpdater::updateCurrentScheme(const TableScheme& currentScheme, const TableScheme& newScheme) {
    TRACE(__FUNCTION__ << "(). Entry point.");
    std::vector<std::string> commands;

    // Карта текущих полей для быстрого доступа
    auto currentFieldsMap = mapFields(currentScheme.fields);
    auto newFieldsMap = mapFields(newScheme.fields);

    std::unordered_set<std::string> toUpdateFields;
    std::unordered_set<std::string> toDeleteFields;

    // Обработка новых и обновленных полей
    for (const auto& newField : newFieldsMap) {
        if (currentFieldsMap.find(newField.first) == currentFieldsMap.end()) {
            addNewFieldToTable(newScheme.name, newField.second, commands);
        } else {
            if (newField.second != currentFieldsMap.at(newField.first)) {
                if (true) { // Проверка, что поле изменилось
                    toUpdateFields.insert(newField.first); // Кажется нужно ещё проверять, что что-то изменилось вообще
                }
            }
        }
    }

    // Удаление старых полей, которые не существуют в новой схеме
    for (const auto& currentField : currentFieldsMap) {
        if (newFieldsMap.find(currentField.first) == newFieldsMap.end()) {
            toDeleteFields.insert(currentField.first);
        }
    }


    std::unordered_set<std::string> affectedKeys;

    // Check if the engine has changed
    std::string currentEngine = "MyISAM";
    if (currentScheme.databaseEngines.find("MySQL") != currentScheme.databaseEngines.end()) {
        currentEngine = currentScheme.databaseEngines.at("MySQL");
    }
    std::string newEngine = "MyISAM";
    if (newScheme.databaseEngines.find("MySQL") != newScheme.databaseEngines.end()) {
        newEngine = newScheme.databaseEngines.at("MySQL");
    }

    bool engineChanged = currentEngine != newEngine;

    // Проверка не изменился ли движок
    // Хранить, новые ключи для добавления
    if (!engineChanged) {
        // Мы знаем, что изменилось. На всякий случай нужно уничтожить "ключи" связанные с этими колонками
        std::unordered_set<std::string> allChanges;
        std::set_union(toUpdateFields.begin(), toUpdateFields.end(), toDeleteFields.begin(), toDeleteFields.end(),
                       std::inserter(allChanges, allChanges.begin()));


        // те ключи которые удалены только из-за изменений, должны вернутся.

        for (const auto& index : currentScheme.indexes) {
            for (const auto& field : index.second) {
                if (allChanges.find(field) != allChanges.end()) {
                    commands.emplace_back("ALTER TABLE " + currentScheme.name + " DROP INDEX " + index.first + ";");
                    affectedKeys.insert(index.first);
                    break;
                }
            }

            if (newScheme.relations.find(index.first) == newScheme.relations.end() && affectedKeys.find(index.first) == affectedKeys.end()) {
                // Ключ надо удалить, его больше не будет.
                commands.emplace_back("ALTER TABLE " + currentScheme.name + " DROP INDEX " + index.first + ";");
            }
        }

        for (const auto& uniqueSet : currentScheme.uniqueFieldsSets) {
            for (const auto& field : uniqueSet.second) {
                if (allChanges.find(field) != allChanges.end()) {
                    commands.emplace_back("ALTER TABLE " + currentScheme.name + " DROP INDEX " + uniqueSet.first + ";");
                    affectedKeys.insert(uniqueSet.first);
                    break;
                }
            }

            if (newScheme.relations.find(uniqueSet.first) == newScheme.relations.end() && affectedKeys.find(uniqueSet.first) == affectedKeys.end()) {
                // Ключ надо удалить, его больше не будет.
                commands.emplace_back("ALTER TABLE " + currentScheme.name + " DROP INDEX " + uniqueSet.first + ";");
            }
        }

        for (const auto& relation : currentScheme.relations) {
            for (const auto& field : relation.second.getLocalFields()) {
                if (allChanges.find(field) != allChanges.end()) {
                    commands.emplace_back("ALTER TABLE " + currentScheme.name + " DROP FOREIGN KEY " + relation.first + ";");
                    affectedKeys.insert(relation.first);
                    break;
                }
            }

            if (newScheme.relations.find(relation.first) == newScheme.relations.end() && affectedKeys.find(relation.first) == affectedKeys.end()) {
                // Ключ надо удалить, его больше не будет.
                commands.emplace_back("ALTER TABLE " + currentScheme.name + " DROP FOREIGN KEY " + relation.first + ";");
            }
        }

        // удалённые, добавленные ключи отдельным сиском
    } else {
        // Удаляем ключи на всякий случай.
        for (const auto& index : currentScheme.indexes) {
            commands.emplace_back("ALTER TABLE " + currentScheme.name + " DROP INDEX " + index.first + ";");
        }
        for (const auto& uniqueSet : currentScheme.uniqueFieldsSets) {
            commands.emplace_back("ALTER TABLE " + currentScheme.name + " DROP INDEX " + uniqueSet.first + ";");
        }
        for (const auto& relation : currentScheme.relations) {
            commands.emplace_back("ALTER TABLE " + currentScheme.name + " DROP FOREIGN KEY " + relation.first + ";");
        }
    }

    // Обновляем колонки.
    for (auto& field : toDeleteFields) {
        commands.emplace_back("ALTER TABLE " + currentScheme.name + " DROP COLUMN " + field + ";");
    }
    for (auto& field : toUpdateFields) {
        updateColumnProperties(newScheme.name, currentFieldsMap.at(field), newFieldsMap.at(field), commands);
    }

    // Change the table engine if necessary
    if (engineChanged) {
        commands.emplace_back("ALTER TABLE " + currentScheme.name + " ENGINE = " + newScheme.databaseEngines.at("MySQL") + ";");
    }

    // Выполнение всех собранных скриптов
    // TODO рассмотреть возможность использования транзакций
    auto poolClient = getSqlPoolClient();
    for (const auto& command : commands) {
        try {
            TRACE(__FUNCTION__ << "(). Command: " << command);
            poolClient->get()->update(command);
        } catch (Exception* ex) {
            TRACE(__FUNCTION__ << "(). Command execution failed: " << ex->getText() << " query: " << command);
            delete ex;
            return -1;
        }
    }

    // Добавляем всё, что нужно через контроллер.
    std::shared_ptr<MySqlDatabaseController> mySqlController = std::dynamic_pointer_cast<MySqlDatabaseController>(databaseController);
    if (mySqlController) {
        if (engineChanged) {
            mySqlController->createIndexes(newScheme.indexes, newScheme.name);
            mySqlController->addUniqueConstraints(newScheme.uniqueFieldsSets, newScheme.fields, newScheme.name);
            mySqlController->buildRelations(newScheme.relations, newScheme.name);
        } else {
            auto indexesToAdd = std::unordered_map<std::string, std::vector<std::string>>();
            for (const auto& index : newScheme.indexes) {
                if (currentScheme.indexes.find(index.first) == currentScheme.indexes.end()) {
                    indexesToAdd[index.first] = index.second;
                } else if (affectedKeys.find(index.first) != affectedKeys.end()) {
                    indexesToAdd[index.first] = index.second;
                }
            }
            mySqlController->createIndexes(indexesToAdd, newScheme.name);

            auto uniquesToAdd = std::unordered_map<std::string, std::vector<std::string>>();
            for (const auto& uniqueKey : newScheme.uniqueFieldsSets) {
                if (currentScheme.indexes.find(uniqueKey.first) == currentScheme.indexes.end()) {
                    uniquesToAdd[uniqueKey.first] = uniqueKey.second;
                } else if (affectedKeys.find(uniqueKey.first) != affectedKeys.end()) {
                    uniquesToAdd[uniqueKey.first] = uniqueKey.second;
                }
            }
            mySqlController->addUniqueConstraints(uniquesToAdd, newScheme.fields, newScheme.name);

            auto relationsToAdd = std::unordered_map<std::string, Relation>();
            for (const auto& relation : newScheme.relations) {
                if (currentScheme.indexes.find(relation.first) == currentScheme.indexes.end()) {
                    relationsToAdd[relation.first] = relation.second;
                } else if (affectedKeys.find(relation.first) != affectedKeys.end()) {
                    relationsToAdd[relation.first] = relation.second;
                }
            }
            mySqlController->buildRelations(relationsToAdd, newScheme.name);
        }
    } else {
        LOG(LERROR, __FUNCTION__ << "() casting to MySqlDatabaseController failed");
    }

    TRACE(__FUNCTION__ << "(). End point.");
    return databaseController->upsertTableVersion(newScheme.name, newScheme.version);
}

std::unordered_map<std::string, StructureField> MySqlSchemeUpdater::mapFields(const std::vector<StructureField>& fields) {
    std::unordered_map<std::string, StructureField> fieldsMap;
    for (const auto& field : fields) {
        fieldsMap[field.get(FIELD_NAME).asString()] = field;
    }
    return fieldsMap;
}

void MySqlSchemeUpdater::addNewFieldToTable(const std::string& tableName, const StructureField& field, std::vector<std::string>& scripts) {
    std::stringstream query;
    // TODO По умолчанию CanBe Empty true
    query << "ALTER TABLE " << tableName <<
          " ADD COLUMN " << field.get(FIELD_NAME).asString() << " " <<
          typesConverter->getMappedType(field.get(FIELD_TYPE).asString(), !field.get(LENGTH_PROPERTY).isInt() ? -1 : field.get(LENGTH_PROPERTY).asInt());

    if (field.get(DEFAULT_VALUE_PROPERTY).isInt() || field.get(DEFAULT_VALUE_PROPERTY).isString() || field.get(DEFAULT_VALUE_PROPERTY).isBool()) {
        query << " DEFAULT " << typesConverter->adaptValueToType(field.get(FIELD_TYPE).asString(), field.get(DEFAULT_VALUE_PROPERTY));
    }
    if (!field.get(CAN_BE_EMPTY_PROPERTY).isBool() || !field.get(CAN_BE_EMPTY_PROPERTY).asBool()) {
        query << " NOT NULL";
    }

    scripts.emplace_back(query.str() + ";");
}

void MySqlSchemeUpdater::updateColumnProperties(const std::string& tableName, const StructureField& oldField,
                                                const StructureField& newField, std::vector<std::string>& scripts) {

    // TODO По умолчанию CanBe Empty true

    std::stringstream modifyQuery;
    modifyQuery << "ALTER TABLE " << tableName << " MODIFY COLUMN " << newField.get(FIELD_NAME).asString();

    // Определение нового типа данных и его длины
    bool typeOrLengthChanged = false;
    std::string newTypeDefinition = typesConverter->getMappedType(newField.get(FIELD_TYPE).asString(), newField.get(LENGTH_PROPERTY).isInt() ? newField.get(LENGTH_PROPERTY).asInt() : -1);

    if (newField.get(FIELD_TYPE).asString() != oldField.get(FIELD_TYPE).asString() ||
        (newField.get(LENGTH_PROPERTY).isInt() && newField.get(LENGTH_PROPERTY).asInt() != oldField.get(LENGTH_PROPERTY).asInt())) {
    }
    modifyQuery << " " << newTypeDefinition;

    // Обработка значения по умолчанию
    bool defaultValueChanged = newField.get(DEFAULT_VALUE_PROPERTY) != oldField.get(DEFAULT_VALUE_PROPERTY);
    if (!newField.get(DEFAULT_VALUE_PROPERTY).isNull()) {
        modifyQuery << " DEFAULT " << typesConverter->adaptValueToType(newField.get(FIELD_TYPE).asString(), newField.get(DEFAULT_VALUE_PROPERTY));
    }

    // Обработка ограничения NOT NULL
    bool notNullChanged = newField.get(CAN_BE_EMPTY_PROPERTY).asBool() != oldField.get(CAN_BE_EMPTY_PROPERTY).asBool();
    if (!newField.get(CAN_BE_EMPTY_PROPERTY).asBool()) {
        modifyQuery << " NOT NULL";
    } else {
        modifyQuery << " NULL";
    }

    modifyQuery << ";";
    // Добавляем запрос на модификацию столбца в список скриптов, если были изменения
    if (typeOrLengthChanged || defaultValueChanged || notNullChanged) {
        scripts.push_back(modifyQuery.str());
    }
}

