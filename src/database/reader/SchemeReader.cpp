#include    "SchemeReader.h"
#include    <Config.h>
#include    <functional>

SchemeReader::SchemeReader() {
    jsonsDirectoryPath = Config::get(JSON_PATH_VARIABLE).empty()
                         ? (fs::path(
#ifdef _WIN32
                    "./jsons"
#else
                    "/etc/sphinx/jsons"
#endif
            )).string() : Config::get(JSON_PATH_VARIABLE);
}

std::string SchemeReader::getDefaultJsonsDirectoryPath() const {
    return jsonsDirectoryPath;
}

std::list<std::shared_ptr<EntityScheme>> SchemeReader::readSchemes(const std::string& directoryPath) {
    auto& cachedSchemes = schemaCache[directoryPath];
    if (!cachedSchemes.empty()) {
        return cachedSchemes;
    }

    fs::path path = directoryPath;
    if (!fs::is_directory(path)) {
        THROW(SystemException, "Provided path is not a directory: " + directoryPath)
    }

    for (const auto& entry : fs::directory_iterator(path)) {
        if (isJsonFile(entry)) {
            cachedSchemes.push_back(readScheme(entry.path()));
        }
    }
    return cachedSchemes;
}

std::shared_ptr<EntityScheme> SchemeReader::readScheme(const fs::path& filePath) {
    try {
        return readEnumTypeScheme(filePath);
    } catch (Exception* e) {
        TRACE(__FUNCTION__ << "(). Scheme is not EnumTypeScheme");
        delete e;
    }

    try {
        return readTableScheme(filePath);
    } catch (Exception* e) {
        TRACE(__FUNCTION__ << "(). Scheme is not TableScheme");
        delete e;
    }

    THROW(SystemException, "Provided path contains invalid scheme");
}

std::shared_ptr<EnumTypeScheme> SchemeReader::readEnumTypeScheme(const fs::path& filePath) {
    if (!isJsonFile(filePath)) {
        THROW(SystemException, "File is not a JSON file. Path: " + filePath.string());
    }

    Json::Value jsonRoot;
    if (!parseJsonFile(filePath.string(), jsonRoot)) {
        THROW(SystemException, "JSON file cannot be parsed. Path: " + filePath.string());
    }

    std::string entityName;
    if (!validateAndExtractName(jsonRoot, entityName) || entityName.empty()) {
        THROW(SystemException, "Entity name is missing or invalid. Path: " + filePath.string());
    }

    int version = jsonRoot[VERSION_PROPERTY].isInt() ? jsonRoot[VERSION_PROPERTY].asInt() : -1;

    bool needProvider = jsonRoot[NEED_PROVIDER_PROPERTY].isBool() ? jsonRoot[NEED_PROVIDER_PROPERTY].asBool() : true;

    std::vector<std::string> values;
    if (!processEntityValues(jsonRoot, values)) {
        THROW(SystemException, "Entity values are missing or invalid. Path: " + filePath.string());
    }

    return std::make_shared<EnumTypeScheme>(entityName, version, needProvider, values);
}

std::shared_ptr<TableScheme> SchemeReader::readTableScheme(const fs::path& filePath) {
    if (!isJsonFile(filePath)) {
        THROW(SystemException, "File is not a JSON file. Path: " + filePath.string());
    }

    Json::Value jsonRoot;
    if (!parseJsonFile(filePath.string(), jsonRoot)) {
        THROW(SystemException, "JSON file cannot be parsed. Path: " + filePath.string());
    }

    std::string entityName;
    if (!validateAndExtractName(jsonRoot, entityName) || entityName.empty()) {
        THROW(SystemException, "Entity name is missing or invalid. Path: " + filePath.string());
    }

    int version = jsonRoot[VERSION_PROPERTY].isInt() ? jsonRoot[VERSION_PROPERTY].asInt() : -1;

    bool needProvider = jsonRoot[NEED_PROVIDER_PROPERTY].isBool() ? jsonRoot[NEED_PROVIDER_PROPERTY].asBool() : true;

    std::vector<StructureField> fields;
    if (!processEntityFields(jsonRoot, fields)) {
        THROW(SystemException, "Entity fields are missing or invalid. Path: " + filePath.string());
    }

    std::unordered_map<std::string, std::vector<std::string>> indexes;
    if (!extractIndexes(jsonRoot, indexes, entityName)) {
        THROW(SystemException, "Entity indexes are missing or invalid. Path: " + filePath.string());
    }

    std::unordered_map<std::string, std::vector<std::string>> uniqueSets;
    if (!extractUniqueSets(jsonRoot, uniqueSets, entityName)) {
        THROW(SystemException, "Entity unique sets are missing or invalid. Path: " + filePath.string());
    }

    std::unordered_map<std::string, Relation> relations;
    if (!extractRelations(jsonRoot, relations, entityName)) {
        THROW(SystemException, "Entity relations are missing or invalid. Path: " + filePath.string());
    }

    std::unordered_map<std::string, std::string> engines;
    if (!extractDatabaseEngines(jsonRoot, engines)) {
        THROW(SystemException, "Database engines are missing or invalid. Path: " + filePath.string());
    }

    return std::make_shared<TableScheme>(entityName, version, needProvider, fields, indexes, uniqueSets, relations, engines);
}

std::unordered_map<std::string, std::list<JsonEntity>> SchemeReader::readAllEntities(const std::string& directory) {
    fs::path directoryPath = directory;
    if (!fs::is_directory(directoryPath)) {
        THROW(SystemException, "Cannot initialize entities: Provided path is not a directory - " + directory);
    }

    std::unordered_map<std::string, std::list<JsonEntity>> allEntities;
    for (const auto& entry : fs::directory_iterator(directoryPath)) {
        if (isJsonFile(entry)) {
            std::list<JsonEntity> parsedEntities = readEntities(entry.path());
            if (!parsedEntities.empty()) {
                allEntities[parsedEntities.front().entityName] = std::move(parsedEntities);
            }
        }
    }
    return allEntities;
}

std::list<JsonEntity> SchemeReader::readEntities(const fs::path& filePath) {
    if (!isJsonFile(filePath)) {
        THROW(SystemException, "File is not a JSON file. Path: " + filePath.string());
    }

    Json::Value jsonRoot;
    if (!parseJsonFile(filePath.string(), jsonRoot)) {
        THROW(SystemException, "JSON cannot be parsed. Path: " + filePath.string());
    }

    std::list<JsonEntity> entities;
    if (!extractEntities(jsonRoot, entities)) {
        THROW(SystemException, "Entities cannot be extracted. Path: " + filePath.string());
    }

    return entities;
}

bool SchemeReader::validateAndExtractName(const Json::Value& jsonRoot, std::string& name) {
    const Json::Value& nameValue = jsonRoot[FIELD_NAME];
    if (nameValue.isString()) {
        name = nameValue.asString();
        return true;
    } else {
        TRACE(__FUNCTION__ << " Error: Name should be a string.");
        return false;
    }
}

bool SchemeReader::isJsonFile(const fs::path& path) {
    return fs::is_regular_file(path) && path.extension() == ".json";
}

bool SchemeReader::parseJsonFile(const std::string& filePath, Json::Value& jsonRoot) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        TRACE(__FUNCTION__ << " Error: Unable to open file " << filePath);
        return false;
    }

    Json::Reader jsonReader;
    if (!jsonReader.parse(file, jsonRoot) || !jsonRoot.isObject()) {
        TRACE(__FUNCTION__ << " Error: Bad JSON in file " << filePath);
        return false;
    }

    return true;
}

bool SchemeReader::processEntityFields(const Json::Value& jsonRoot, std::vector<StructureField>& fields) {
    const Json::Value& jsonFields = jsonRoot[FIELDS_PROPERTY];
    if (!jsonFields.isArray() || jsonFields.empty()) {
        TRACE(__FUNCTION__ << "(). Error: Entity has no fields.");
        return false;
    }

    // Adding the required id
    fields.emplace_back(StructureField({
                                       {FIELD_NAME, KEY_FIELD},
                                       {FIELD_TYPE, INT_JSON_TYPE},
                                       {CAN_BE_EMPTY_PROPERTY, false}
                                       }));

    // Process all fields
    for (const auto& jsonField : jsonFields) {
        if (!processField(jsonField, fields)) {
            TRACE(__FUNCTION__ << "(). Error: Cannot parse field.");
            return false;
        }
    }
    return true;
}


bool SchemeReader::processField(const Json::Value& jsonField, std::vector<StructureField>& fields) {
    if (!jsonField.isObject() || !jsonField[FIELD_NAME].isString() || jsonField[FIELD_NAME].asString().empty()) {
        TRACE(__FUNCTION__ << "(). Error: Invalid field.");
        return false;
    }

    std::initializer_list<std::pair<std::string, Json::Value>> properties = {
            {FIELD_NAME, jsonField[FIELD_NAME].asString()},
            {FIELD_TYPE, jsonField[FIELD_TYPE].asString()}
    };

    StructureField structureField(properties);

    // Dynamically add additional properties if they exist
    const std::string fieldType = jsonField[FIELD_TYPE].asString();
    addFieldProperty(structureField, jsonField, LENGTH_PROPERTY, fieldType);
    addFieldProperty(structureField, jsonField, CAN_BE_EMPTY_PROPERTY, fieldType);
    addFieldProperty(structureField, jsonField, DEFAULT_VALUE_PROPERTY, fieldType);

    fields.push_back(std::move(structureField));
    return true;
}

bool SchemeReader::processEntityValues(const Json::Value& jsonRoot, std::vector<std::string>& values) {
    const Json::Value& jsonValues = jsonRoot["Values"];
    if (!jsonValues.isArray() || jsonValues.empty()) {
        return false;
    }

    for (const auto& jsonValue : jsonValues) {
        if (!jsonValue.isString()) {
            TRACE(__FUNCTION__ << "(). Error: Cannot parse value.");
            return false;
        }
        values.push_back(jsonValue.asString());
    }
    return true;
}

void SchemeReader::addFieldProperty(StructureField& field, const Json::Value& jsonField, const std::string& propertyName, const std::string& type) {
    std::unordered_map<std::string, std::function<bool(const Json::Value&)>> validators = {
            {LENGTH_PROPERTY, [](const Json::Value& value) { return value.isInt(); }},
            {CAN_BE_EMPTY_PROPERTY, [](const Json::Value& value) { return value.isBool(); }},
            {DEFAULT_VALUE_PROPERTY, [type](const Json::Value& value) {
                if (value.isNull()) {
                    return true;
                } else if (type == INT_JSON_TYPE) {
                    return value.isInt();
                } else if (type == FLOAT_JSON_TYPE) {
                    return value.isDouble();
                } else if (type == STRING_JSON_TYPE) {
                    return value.isString();
                } else if (type == BOOLEAN_JSON_TYPE) {
                    return value.isBool();
                } else if (type == BINARY_JSON_TYPE) {
                    return value.isString();
                } else if (type == DATE_JSON_TYPE) {
                    return value.isString();
                } else if (type == DATETIME_JSON_TYPE) {
                    return value.isString();
                } else {
                    return value.isString();
                }
            }}
    };

    auto validator = validators.find(propertyName);
    if (validator != validators.end()) {
        if (validator->second(jsonField[propertyName])) {
            field.put(propertyName, jsonField[propertyName]);
        }
    }
}

bool SchemeReader::extractRelations(const Json::Value& jsonRoot, std::unordered_map<std::string, Relation>& relations, const std::string& schemeName) {
    Json::Value relationsJson = jsonRoot[RELATIONS_PROPERTY];
    if (relationsJson.isArray()) {
        for (const auto& relation : relationsJson) {
            if (relation.isObject()) {

                std::vector<std::string> localFields;
                if (relation[RELATIONS_LOCAL_FIELDS].isArray()) {
                    for (const auto& field : relation[RELATIONS_LOCAL_FIELDS]) {
                        if (field.isString()) {
                            localFields.push_back(field.asString());
                        } else {
                            THROW(SystemException, "LocalFields can contains only strings!")
                        }
                    }
                } else {
                    continue;
                }

                std::string targetName;
                std::vector<std::string> targetFields;
                if (relation[RELATIONS_TARGET].isObject()) {
                    auto target = relation[RELATIONS_TARGET];
                    if (target[RELATIONS_TARGET_NAME].isString()) {
                        targetName = target[RELATIONS_TARGET_NAME].asString();
                    } else {
                        THROW(SystemException, "Target entity name should be a string!")
                    }

                    if (target[RELATIONS_TARGET_FIELDS].isArray()) {
                        for (const auto& field : target[RELATIONS_TARGET_FIELDS]) {
                            if (field.isString()) {
                                targetFields.push_back(field.asString());
                            } else {
                                THROW(SystemException, "TargetFields can contains only strings!")
                            }
                        }
                    } else {
                        continue;
                    }
                } else {
                    continue;
                }

                Relation::RelationType onDelete = Relation::RESTRICT;
                Relation::RelationType onUpdate = Relation::RESTRICT;
                if (relation.isMember(RELATIONS_ON_DELETE) && relation[RELATIONS_ON_DELETE].isString()) {
                    onDelete = Relation::strToRelationType(relation[RELATIONS_ON_DELETE].asString());
                }

                if (relation.isMember(RELATIONS_ON_UPDATE) && relation[RELATIONS_ON_UPDATE].isString()) {
                    onUpdate = Relation::strToRelationType(relation[RELATIONS_ON_UPDATE].asString());
                }

                // Добавляем внешний ключ в коллекцию
                std::string key = StringTools::toLower(schemeName) + "_" + StringTools::toLower(targetName) + "_"
                                  + StringTools::toLower(StringTools::concat(localFields.cbegin(), localFields.cend(), '_')) + "_"
                                  + StringTools::toLower(StringTools::concat(targetFields.cbegin(), targetFields.cend(), '_')) + "_fkey";

                relations.insert({key, {localFields, targetName, targetFields, onDelete, onUpdate}});
            }
        }
    }
    return true;
}

bool SchemeReader::extractIndexes(const Json::Value& jsonRoot, std::unordered_map<std::string, std::vector<std::string>>& indexes, const std::string& schemeName) {
    Json::Value indexesJson = jsonRoot[INDEXES_PROPERTY];
    if (indexesJson.isArray()) {
        for (const auto& index : indexesJson) {
            if (index.isArray()) {
                std::vector<std::string> fields;
                for (const auto& field : index) {
                    if (field.isString()) {
                        fields.emplace_back(field.asString());
                    }
                }
                std::string keyName = StringTools::toLower(schemeName) + "_" + StringTools::toLower(StringTools::concat(fields.begin(), fields.end(), '_')) + "_idx";
                indexes.insert({keyName, std::move(fields)});
            }
        }
    }
    return true;
}

bool SchemeReader::extractUniqueSets(const Json::Value& jsonRoot, std::unordered_map<std::string, std::vector<std::string>>& uniqueSets, const std::string& schemeName) {
    Json::Value setsJson = jsonRoot[UNIQUE_FIELDS_SETS_PROPERTY];
    if (setsJson.isArray()) {
        for (const auto& set : setsJson) {
            if (set.isArray()) {
                std::vector<std::string> fields;
                for (const auto& field : set) {
                    if (field.isString()) {
                        fields.emplace_back(field.asString());
                    }
                }
                std::string keyName = StringTools::toLower(schemeName) + "_" + StringTools::toLower(StringTools::concat(fields.begin(), fields.end(), '_')) + "_ukey";
                uniqueSets.insert({keyName, std::move(fields)});
            }
        }
    }
    return true;
}

bool SchemeReader::extractDatabaseEngines(const Json::Value& jsonRoot, std::unordered_map<std::string, std::string>& engines) {
    Json::Value enginesJson = jsonRoot[ENGINES_PROPERTY];
    if (enginesJson.isArray()) {
        for (const auto& engine : enginesJson) {
            if (engine.isObject()) {
                if (engine["MySQL"].isString()) {
                    engines.insert({"MySQL", engine["MySQL"].asString()});
                }
            }
        }
    }
    return true;
}

bool SchemeReader::extractEntities(const Json::Value& jsonRoot, std::list<JsonEntity>& entities) {
    const Json::Value& defaultEntities = jsonRoot[DEFAULT_VALUES_PROPERTY];
    if (!defaultEntities.isArray()) {
        TRACE(__FUNCTION__ << "(). Error: DefaultValues is not an array!");
        return false;
    }

    std::string entityName;
    if (!validateAndExtractName(jsonRoot, entityName)) {
        TRACE(__FUNCTION__ << "(). Error: Entity name not valid!");
        return false;
    }

    static auto jsonContainsKey = [](const Json::Value &json, const std::string &key){
        Json::Value::Members members = json.getMemberNames();
        return std::find(members.cbegin(), members.cend(), key) != members.cend();
    };

    auto fields = EntitiesStructureCache::getSingletonInstance()->getFieldsTypes( entityName );
    for (const auto& entityValues : defaultEntities) {
        std::unordered_map<std::string, Json::Value> valueMap;
        int key = -1;
        for (const auto& field : fields) {
            if (!jsonContainsKey(entityValues, field.first)) {
                continue; // skip ID field
            } else if (entityValues[field.first].isNull()) {
                valueMap[field.first] = Json::nullValue;
            } else if (field.second == INT_JSON_TYPE) {
                if (field.first == KEY_FIELD) {
                    if (entityValues[field.first].isInt()) {
                        key = entityValues[field.first].asInt();
                    } else {
                        continue;
                    }
                }
                valueMap[field.first] = {entityValues[field.first].asInt()};
            } else if (field.second == FLOAT_JSON_TYPE) {
                valueMap[field.first] = {entityValues[field.first].asDouble()};
            } else if (field.second == STRING_JSON_TYPE) {
                valueMap[field.first] = {entityValues[field.first].asString()};
            } else if (field.second == BOOLEAN_JSON_TYPE) {
                valueMap[field.first] = {entityValues[field.first].asBool()};
            } else if (field.second == BINARY_JSON_TYPE) {
                valueMap[field.first] = {entityValues[field.first].asString()};
            } else if (field.second == DATE_JSON_TYPE) {
                valueMap[field.first] = {entityValues[field.first].asString()};
            } else if (field.second == DATETIME_JSON_TYPE) {
                valueMap[field.first] = {entityValues[field.first].asString()};
            } else {
                valueMap[field.first] = {entityValues[field.first].asString()};
            }
        }
        entities.emplace_back(entityName, key, valueMap);
    }
    return true;
}
