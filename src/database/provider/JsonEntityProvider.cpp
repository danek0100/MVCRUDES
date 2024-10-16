#include    "JsonEntityProvider.h"
#include    "./database/cache/EntitiesCache.h"


JsonEntityProvider::JsonEntityProvider(): Singleton<JsonEntityProvider>(),
        JsonProviderWithFilteredSpeaker<JsonEntity>(),
        database(DatabaseManager::getCrudController().get()) {
    TRACE(__FUNCTION__ << "() entry point")
    TRACE(__FUNCTION__ << "() end point")
}

void JsonEntityProvider::createImpl(std::list<JsonEntity>& objects) {
    TRACE(__FUNCTION__ << "() entry point");
    double startTime = Timer::getTime();

    for (auto& object : objects) {
        fillEntity(object);
        checkEntity(object);
    }
    TRACE(__FUNCTION__ << "() objects preprocessed. " << (Timer::getTime()-startTime) << " secs.");

    database->addEntities(objects);
    TRACE(__FUNCTION__ << "() end point. " << (Timer::getTime()-startTime) << " secs.");
}

std::list<JsonEntity> JsonEntityProvider::readImpl(const std::string& entityName, const Json::Value& filter, const std::set<std::string>& fields, bool forceRead) {
    TRACE(__FUNCTION__ << "() entry point")
    double startTime = Timer::getTime();
    std::list<JsonEntity> result;
    JsonEntityFilter jsonFilter(filter);

    if (!forceRead && EntitiesCache::getSingletonInstance()->hasCache(entityName)) {
        for (const auto &item: EntitiesCache::getSingletonInstance()->getCache(entityName)) {
            if (jsonFilter.isFiltered(item.toJson())) {
                if (fields.empty()) {
                    result.push_back(item);
                } else {
                    std::unordered_map<std::string, Json::Value> filteredFields;
                    for (auto& field : fields) {
                        if (item.value.find(field) != item.value.end()) {
                            filteredFields[field] = item.value.at(field);
                        }
                    }
                    result.emplace_back(item.entityName, item.key, filteredFields);
                }
            }
        }
    } else {
        result = database->loadEntities(entityName, filter, fields);
    }

    TRACE(__FUNCTION__ << "() end point. result.size:[" << result.size() << "]. " << (Timer::getTime()-startTime) << " secs.")
    return result;
}

std::list<JsonEntity> JsonEntityProvider::updateImpl(std::list<JsonEntity>& objects) {
    TRACE(__FUNCTION__ << "() entry point")
    double startTime = Timer::getTime();
    for (const auto& object : objects) {
        checkEntity(object);
    }
    TRACE(__FUNCTION__ << "() objects checked. " << (Timer::getTime()-startTime) << " secs.");

    std::list<JsonEntity> result = database->updateEntities(objects);
    TRACE(__FUNCTION__ << "() end point. " << (Timer::getTime()-startTime) << " secs.")
    return result;
}

std::list<JsonEntity> JsonEntityProvider::removeImpl(std::list<JsonEntity>& objects) {
    TRACE(__FUNCTION__ << "() entry point")
    double startTime = Timer::getTime();
    return database->removeEntities(objects);
    TRACE(__FUNCTION__ << "() end point. " << (Timer::getTime()-startTime) << " secs.")
}

void JsonEntityProvider::notifyProviderAboutChanges(uint32_t event, const JsonEntity& object) {
    TRACE(__FUNCTION__ << "() entry point")
    this->notifySubscribers(event, object);
    TRACE(__FUNCTION__ << "() end point")
}

void JsonEntityProvider::fillEntity(JsonEntity &object) {
    TRACE(__FUNCTION__ << "() entry point")

    std::vector<StructureField> allFields = EntitiesStructureCache::getSingletonInstance()->getFields(object.entityName);

    for (const auto& field : allFields) {
        auto fieldName = field.get(FIELD_NAME).asString();
        if (fieldName == KEY_FIELD) continue;
        // Проверяем, существует ли поле в объекте
        if (object.value.find(fieldName) == object.value.end()) {
            bool filled = false;
            // Поле отсутствует, пытаемся заполнить его значением по умолчанию
            if (field.get(FIELD_TYPE).asString() == INT_JSON_TYPE && field.get(DEFAULT_VALUE_PROPERTY).isInt()) {
                object.value[fieldName] = {field.get(DEFAULT_VALUE_PROPERTY).asInt()};
                filled = true;
            } if (field.get(FIELD_TYPE).asString() == FLOAT_JSON_TYPE && field.get(DEFAULT_VALUE_PROPERTY).isDouble()) {
                object.value[fieldName] = {field.get(DEFAULT_VALUE_PROPERTY).asDouble()};
                filled = true;
            } else if (field.get(FIELD_TYPE).asString() == STRING_JSON_TYPE && field.get(DEFAULT_VALUE_PROPERTY).isString()) {
                object.value[fieldName] = {field.get(DEFAULT_VALUE_PROPERTY).asString()};
                filled = true;
            } else if (field.get(FIELD_TYPE).asString() == BOOLEAN_JSON_TYPE && field.get(DEFAULT_VALUE_PROPERTY).isBool()) {
                object.value[fieldName] = {field.get(DEFAULT_VALUE_PROPERTY).asBool()};
                filled = true;
            } else if (field.get(FIELD_TYPE).asString() == BINARY_JSON_TYPE && field.get(DEFAULT_VALUE_PROPERTY).isString()) {
                object.value[fieldName] = {field.get(DEFAULT_VALUE_PROPERTY).asString()};
                filled = true;
            } else if (field.get(FIELD_TYPE).asString() == DATE_JSON_TYPE && field.get(DEFAULT_VALUE_PROPERTY).isString()) {
                object.value[fieldName] = {field.get(DEFAULT_VALUE_PROPERTY).asString()};
                filled = true;
            } else if (field.get(FIELD_TYPE).asString() == DATETIME_JSON_TYPE && field.get(DEFAULT_VALUE_PROPERTY).isString()) {
                object.value[fieldName] = {field.get(DEFAULT_VALUE_PROPERTY).asString()};
                filled = true;
            } else if (EntitiesStructureCache::getSingletonInstance()->isTypeEnum(field.get(FIELD_TYPE).asString())) {
                std::vector<std::string> values = EntitiesStructureCache::getSingletonInstance()->getEnumValues(field.get(FIELD_TYPE).asString());
                Json::Value defaultValue = field.get(DEFAULT_VALUE_PROPERTY);
                if (defaultValue.isString() && std::find(values.begin(), values.end(), defaultValue.asString()) != values.end()) {
                    object.value[fieldName] = {defaultValue.asString()};
                    filled = true;
                }
            }

            if (!filled && !field.get(CAN_BE_EMPTY_PROPERTY).asBool()) {
                TRACE(__FUNCTION__ << "() found that significant value is absent.")
                THROW(SqlException, field.get(FIELD_NAME).asString() << " cannot be absent");
            }
        }
    }
    TRACE(__FUNCTION__ << "() end point")
}

void JsonEntityProvider::checkEntity(const JsonEntity &object) {
    TRACE(__FUNCTION__ << "() entry point");

    std::vector<std::string> toUniqueCheck;
    auto fields = EntitiesStructureCache::getSingletonInstance()->getFields(object.entityName);

    for (const auto& field : fields) {
        auto fieldName = field.get(FIELD_NAME).asString();
        auto it = object.value.find(fieldName);
        if (it == object.value.end()) continue;

        const auto& fieldValue = it->second;

        if (field.get(CAN_BE_EMPTY_PROPERTY).isBool() && !field.get(CAN_BE_EMPTY_PROPERTY).asBool()
                && fieldValue.isString() && StringTools::trim(fieldValue.asString()).empty()) {
            TRACE(__FUNCTION__ << "() found that value empty, but cannot be.")
            THROW(SqlException, fieldName + " cannot be empty");
        }

        if (field.get(LENGTH_PROPERTY).isInt() && fieldValue.isString() && fieldValue.asString().length() > field.get(LENGTH_PROPERTY).asInt()) {
            TRACE(__FUNCTION__ << "() found that length of value greater than max. Length: " << fieldValue.asString().length() << ", max: " << field.get(LENGTH_PROPERTY).asInt())
            THROW(SqlException, fieldName + " length greater than max. Length: " + std::to_string(fieldValue.asString().length()) + ", max: " + std::to_string(field.get(LENGTH_PROPERTY).asInt()));
        }

        // TODO
/*        if (field.get(IS_UNIQUE_PROPERTY).isBool() && field.get(IS_UNIQUE_PROPERTY).asBool() && fieldName != KEY_FIELD) {
            toUniqueCheck.push_back(fieldName);
        }*/
    }

    if (!toUniqueCheck.empty()) {
        TRACE(__FUNCTION__ << "() checking on unique");
        checkUniqueFields(object, toUniqueCheck);
    }
    TRACE(__FUNCTION__ << "() end point");
}

void JsonEntityProvider::checkUniqueFields(const JsonEntity &object, const std::vector<std::string>& fields)
{
    Json::Value filter(Json::ValueType::objectValue);
    filter[JsonEntityFilter::FILTERS] = Json::Value(Json::ValueType::arrayValue);

    Json::Value fieldsFilter;
    fieldsFilter[JsonEntityFilter::FILTERS] = Json::Value(Json::ValueType::arrayValue);
    for (const auto& fieldName : fields) {
        Json::Value fieldFilter(Json::ValueType::objectValue);
        fieldFilter[JsonEntityFilter::FIELD_NAME] = fieldName;
        fieldFilter[JsonEntityFilter::TYPE] = JsonEntityFilter::EQ;
        fieldFilter[JsonEntityFilter::VALUE] = object.value.at(fieldName);
        fieldsFilter[JsonEntityFilter::FILTERS].append(fieldFilter);
    }

    fieldsFilter[JsonEntityFilter::TYPE] = JsonEntityFilter::OR;
    filter[JsonEntityFilter::FILTERS].append(fieldsFilter);

    Json::Value filterId(Json::ValueType::objectValue);
    std::string id = "ID";
    filterId[JsonEntityFilter::FIELD_NAME] = id;
    filterId[JsonEntityFilter::TYPE] = JsonEntityFilter::NE;
    filterId[JsonEntityFilter::VALUE] = object.key;

    filter[JsonEntityFilter::FILTERS].append(filterId);
    filter[JsonEntityFilter::TYPE] = JsonEntityFilter::AND;

    if (!readImpl(object.entityName, filter).empty()) {
        TRACE(__FUNCTION__ << "() not unique value found.")
        THROW(SqlException, "not unique value found.");
    }
}
