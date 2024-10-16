#include    <include/database/controllers/postgresql/PostgreSqlCrudController.h>

PostgreSqlCrudController::PostgreSqlCrudController(std::shared_ptr<ITypesConverter> typesConverter) :
                                                                            typesConverter(std::move(typesConverter)) {}

PostgreSqlCrudController::~PostgreSqlCrudController() = default;

// TODO simplify in future for testing
std::list<JsonEntity> PostgreSqlCrudController::loadEntities(const std::string& tableName, const Json::Value& filter, const std::set<std::string>& fields = {}) {
    TRACE(__FUNCTION__ << "(). Entry point.");
    SqlPoolClient client;
    SqlResult sqlResult;
    std::list<JsonEntity> result;

    auto allFields = EntitiesStructureCache::getSingletonInstance()->getFieldsTypes( tableName );

    std::ostringstream query;
    query << "SELECT ";
    if (fields.empty()) {
        query << "*";
    } else {
        if (fields.find(KEY_FIELD) == fields.end()) {
            query << KEY_FIELD << ", ";
        }
        for (const auto& field : fields) {
            query << field << ", ";
        }
       query.seekp(-2, query.cur);
    }
    query << " FROM " << tableName;

    try {
        if (!filter.isNull()) {
            std::string whereClause = buildSelectWhereClause(tableName, filter);
            if (!whereClause.empty()) {
                query << " WHERE " << whereClause;
            }
            query << buildSelectOrderByClause(filter);
        }
        query << ";";

        client.get()->query(query.str(), sqlResult);
        while (sqlResult.next()) {
            std::unordered_map<std::string, Json::Value> value;
            int id = sqlResult.getInt(KEY_FIELD);

            if (fields.empty()) {
                for (const auto& field : allFields) {
                    if (!sqlResult.isDataAvailable(field.first)) {
                        value[field.first] = Json::nullValue;
                    } else if (field.second == INT_JSON_TYPE) {
                        value[field.first] = {sqlResult.getInt(field.first)};
                    } else if (field.second == FLOAT_JSON_TYPE) {
                        value[field.first] = {sqlResult.getFloat(field.first)};
                    } else if (field.second == STRING_JSON_TYPE) {
                        value[field.first] = {sqlResult.getString(field.first)};
                    } else if (field.second == BOOLEAN_JSON_TYPE) {
                        value[field.first] = {sqlResult.getInt(field.first) == 1};
                    } else if (field.second == BINARY_JSON_TYPE) {
                        value[field.first] = {sqlResult.getRaw(field.first).toRawHex()};
                    } else if (field.second == DATE_JSON_TYPE) {
                        value[field.first] = {sqlResult.getDate(field.first).get()};
                    } else if (field.second == DATETIME_JSON_TYPE) {
                        value[field.first] = {sqlResult.getDatetime(field.first).get()};
                    } else if (EntitiesStructureCache::getSingletonInstance()->isTypeEnum(field.second)) {
                        value[field.first] = {sqlResult.getString(field.first)};
                    }
                }
            } else {
                for (const auto& field : fields) {
                    if (!sqlResult.isDataAvailable(field)) {
                        value[field] = Json::nullValue;
                    } else if (allFields[field] == INT_JSON_TYPE) {
                        value[field] = {sqlResult.getInt(field)};
                    } else if (allFields[field] == FLOAT_JSON_TYPE) {
                        value[field] = {sqlResult.getFloat(field)};
                    } else if (allFields[field] == STRING_JSON_TYPE) {
                        value[field] = {sqlResult.getString(field)};
                    } else if (allFields[field] == BOOLEAN_JSON_TYPE) {
                        value[field] = {sqlResult.getInt(field) == 1};
                    } else if (allFields[field] == BINARY_JSON_TYPE) {
                        value[field] = {sqlResult.getRaw(field).toRawHex()};
                    } else if (allFields[field] == DATE_JSON_TYPE) {
                        value[field] = {sqlResult.getDate(field).get()};
                    } else if (allFields[field] == DATETIME_JSON_TYPE) {
                        value[field] = {sqlResult.getDatetime(field).get()};
                    } else if (EntitiesStructureCache::getSingletonInstance()->isTypeEnum(allFields[field])) {
                        value[field] = {sqlResult.getString(field)};
                    }
                }
            }
            result.emplace_back(tableName, id, value);
        }
    } catch (Exception* ex) {
        TRACE(__FUNCTION__ << "(). Error! " << ex->getText() << " query: " << query.str());
        delete ex;
        THROW(SqlException, "Read failed.");
    }

    TRACE(__FUNCTION__ << "(). End point.");
    return result;
}

// TODO simplify in future for testing
void PostgreSqlCrudController::addEntities(std::list<JsonEntity>& objects) {
    TRACE(__FUNCTION__ << "(). Entry point.");

    SqlPoolClient client;
    std::list<JsonEntity> result;

    std::unordered_map<std::string, std::list<std::list<JsonEntity>::iterator>> typeSeparation;
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        typeSeparation[it->entityName].push_back(it);
    }

    for (auto& typedEntity : typeSeparation) {
        auto fieldsTypes = EntitiesStructureCache::getSingletonInstance()->getFieldsTypes(typedEntity.first);

        std::ostringstream query;
        query << "INSERT INTO " << typedEntity.first << " (";

        std::vector<std::string> columns;
        for (const auto& pair: typedEntity.second.front()->value) {
            columns.push_back(pair.first);
        }

        query << join(columns, ", ") << ") VALUES ";

        std::vector<std::string> rows;
        for (const auto& object: typedEntity.second) {
            std::ostringstream values;
            for (const auto& col: columns) {
                if (values.tellp() > 0) {
                    values << ", ";
                }
                values << typesConverter->adaptValueToType(fieldsTypes[col], object->value.at(col));
            }
            rows.push_back("(" + values.str() + ")");
        }

        query << join(rows, ", ") << ";";

        try {
            client.get()->update(query.str());

            if (typedEntity.second.front()->key < 0) {
                int lastId = client.get()->queryLastAutoId(typedEntity.first.c_str());

                int firstId = lastId - (int)typedEntity.second.size() + 1;
                for (auto& object : typedEntity.second) {
                    object->key = firstId++;
                    object->value[KEY_FIELD] = {object->key};
                }
            }
        } catch (Exception* ex) {
            TRACE(__FUNCTION__ << "(). Error! " << ex->getText() << " query: " << query.str());
            delete ex;
            THROW(SqlException, "Add failed.");
        }
    }
    TRACE(__FUNCTION__ << "(). End point.");
}

// TODO simplify in future for testing
std::list<JsonEntity> PostgreSqlCrudController::updateEntities(std::list<JsonEntity>& objects) {
    TRACE(__FUNCTION__ << "(). Entry point.");

    SqlPoolClient client;
    std::list<JsonEntity> result;

    std::unordered_map<std::string, std::list<std::list<JsonEntity>::iterator>> typeSeparation;
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        typeSeparation[it->entityName].push_back(it);
    }

    for (auto& typedEntity : typeSeparation) {
        auto fieldsTypes = EntitiesStructureCache::getSingletonInstance()->getFieldsTypes(typedEntity.first);

        std::unordered_map<std::string, std::list<std::list<JsonEntity>::iterator>> fieldsSets;
        for (auto& object : typedEntity.second) {
            std::string fieldsSet;
            for (const auto& field : fieldsTypes) {
                if (object->value.find(field.first) != object->value.end()) {
                    fieldsSet += "0";
                } else {
                    fieldsSet += "1";
                }
            }
            fieldsSets[fieldsSet].push_back(object);
        }

        for (auto& entities : fieldsSets) {
            std::ostringstream query;
            query << "UPDATE " << typedEntity.first << " SET ";

            std::vector<std::string> idsStr;
            Json::Value idsArray = Json::Value(Json::ValueType::arrayValue);

            std::map<std::string, std::ostringstream> updates;
            for (const auto& object : entities.second) {
                for (const auto& pair : object->value) {
                    if (updates.find(pair.first) == updates.end() || updates[pair.first].str().empty()) {
                        updates[pair.first] << " CASE ID ";
                    }
                    updates[pair.first] << "WHEN " << object->key << " THEN " << typesConverter->adaptValueToType(fieldsTypes[pair.first], pair.second) << " ";
                }
                idsStr.push_back(std::to_string(object->key));
                idsArray.append({object->key});
            }

            bool isFirst = true;
            for (const auto& update : updates) {
                if (!isFirst) {
                    query << ", ";
                }
                query << update.first << " = " << update.second.str() << " END";
                isFirst = false;
            }

            query << " WHERE ID IN (" << join(idsStr, ", ") << ")";

            try {
                client.get()->update(query.str());
            } catch (Exception* ex) {
                TRACE(__FUNCTION__ << "(). Error! " << ex->getText() << " query: " << query.str());
                delete ex;
                THROW(SqlException, "Update failed.");
            }

            Json::Value jsonData;
            jsonData[JsonEntityFilter::FIELD_NAME] = KEY_FIELD;
            jsonData[JsonEntityFilter::TYPE] = JsonEntityFilter::IN_;
            jsonData[JsonEntityFilter::VALUES] = idsArray;

            auto entitiesToAdd = loadEntities(typedEntity.first, jsonData);
            result.insert(result.end(), entitiesToAdd.begin(), entitiesToAdd.end());
        }
    }
    TRACE(__FUNCTION__ << "(). End point.");
    return result;
}

// TODO simplify in future for testing
std::list<JsonEntity> PostgreSqlCrudController::removeEntities(std::list<JsonEntity>& objects) {
    TRACE(__FUNCTION__ << "(). Entry point. objects.size:[" << objects.size() <<"]");
    std::list<JsonEntity> result;
    SqlPoolClient client;

    std::unordered_map<std::string, std::list<std::list<JsonEntity>::iterator>> typeSeparation;
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        typeSeparation[it->entityName].push_back(it);
    }

    for (auto& typedEntity : typeSeparation) {
        auto types = EntitiesStructureCache::getSingletonInstance()->getFieldsTypes(typedEntity.first);

        Json::Value idsArray = Json::Value(Json::ValueType::arrayValue);
        std::ostringstream query;
        query << "DELETE FROM " << typedEntity.first << " WHERE ID IN ( ";
        for (const auto& object : typedEntity.second) {
            query << typesConverter->adaptValueToType(types[KEY_FIELD], {object->key}) << ", ";
            idsArray.append({object->key});
        }
        query.seekp(-2, query.cur);
        query << " );";

        Json::Value jsonData;
        jsonData[JsonEntityFilter::FIELD_NAME] = KEY_FIELD;
        jsonData[JsonEntityFilter::TYPE] = JsonEntityFilter::IN_;
        jsonData[JsonEntityFilter::VALUES] = idsArray;

        auto entitiesToAdd = loadEntities(typedEntity.first, jsonData);

        try {
            client.get()->update(query.str());
            result.insert(result.end(), entitiesToAdd.begin(), entitiesToAdd.end());
        } catch (Exception* ex) {
            TRACE(__FUNCTION__ << "(). Error! " << ex->getText() << " query: " << query.str());
            delete ex;
            THROW(SqlException, "Remove failed.");
        }
    }
    TRACE(__FUNCTION__ << "(). End point. result.size:[" << result.size() <<"]");
    return result;
}

std::string PostgreSqlCrudController::buildSelectWhereClause(const std::string& entityName, const Json::Value& filter) {
    if (filter.isNull()) return "";

    std::stringstream whereClause;

    if (!filter[JsonEntityFilter::FILTERS].isNull()) {
        if (filter[JsonEntityFilter::FILTERS].empty()) return "";

        std::string type = filter[JsonEntityFilter::TYPE].asString();
        std::vector<std::string> conditions;

        for (const auto &item: filter[JsonEntityFilter::FILTERS]) {
            conditions.push_back(buildSelectWhereClause(entityName, item));
        }

        std::string connector = type == JsonEntityFilter::AND ? " AND " : " OR ";
        whereClause << "(" << join(conditions, connector) << ")";
    } else {
        std::string fieldName = filter[JsonEntityFilter::FIELD_NAME].asString();
        std::string type = filter[JsonEntityFilter::TYPE].asString();
        std::string sqlOperator = getSqlOperator(type);

        auto types = EntitiesStructureCache::getSingletonInstance()->getFieldsTypes(entityName);

        if (type == JsonEntityFilter::IN_) {
            if (filter[JsonEntityFilter::VALUES].empty()) {
                return "FALSE";
            }
            whereClause << fieldName << " IN (";
            for (const auto& value: filter[JsonEntityFilter::VALUES]) {
                whereClause << typesConverter->adaptValueToType(types[fieldName], value) << ",";
            }
            whereClause.seekp(-1, whereClause.cur);
            whereClause << ")";
        } else if (type == JsonEntityFilter::NIN) {
            if (filter[JsonEntityFilter::VALUES].empty()) {
                return "TRUE";
            }
            whereClause << fieldName << " NOT IN (";
            for (const auto& value: filter[JsonEntityFilter::VALUES]) {
                whereClause << typesConverter->adaptValueToType(types[fieldName], value) << ",";
            }
            whereClause.seekp(-1, whereClause.cur);
            whereClause << ")";
        } else {
            whereClause << fieldName << " " << sqlOperator << " " << typesConverter->adaptValueToType(types[fieldName], filter["value"]);
        }
    }
    return whereClause.str();
}


std::string PostgreSqlCrudController::buildSelectOrderByClause(const Json::Value& filter){
    if(filter[JsonEntityFilter::SORTS].isNull()) return "";
    std::string result;

    result.append("ORDER BY ");
    for(const auto& sortFilter : filter[JsonEntityFilter::SORTS]){
        result.append(sortFilter[JsonEntityFilter::FIELD_NAME].asString());
        result.append((sortFilter[JsonEntityFilter::IS_ASCENDING].asBool()) ? "" : " DESC");
        result.append(", ");
    }
    result.pop_back();
    result.pop_back();

    return result;
}


std::string PostgreSqlCrudController::join(const std::vector<std::string>& elements, const std::string& connector) {
    std::ostringstream result;
    for (size_t i = 0; i < elements.size(); ++i) {
        if (i > 0) result << connector;
        result << elements[i];
    }
    return result.str();
}

std::string PostgreSqlCrudController::getSqlOperator(const std::string& type) {
    if (type == JsonEntityFilter::EQ) return "=";
    else if (type == JsonEntityFilter::NE) return "!=";
    else if (type == JsonEntityFilter::LT) return "<";
    else if (type == JsonEntityFilter::LE) return "<=";
    else if (type == JsonEntityFilter::GT) return ">";
    else if (type == JsonEntityFilter::GE) return ">=";
    else if (type == JsonEntityFilter::IN_) return "IN";
    else if (type == JsonEntityFilter::NIN) return "NOT IN";
    else if (type == JsonEntityFilter::IS) return "IS";
    else if (type == JsonEntityFilter::NIS) return "IS NOT";
    return "";
}
