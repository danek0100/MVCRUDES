#include    "JsonEntity.h"

JsonEntity::JsonEntity(std::string _entityName, int _key, std::unordered_map<std::string, Json::Value> _value):
                                                entityName(std::move(_entityName)),
                                                key(_key),
                                                value(std::move(_value)) {}

JsonEntity::JsonEntity(const Json::Value& data) {
    TRACE(__FUNCTION__ << " entry point.");
    if (!data.isObject() || data.empty()) {
        TRACE(__FUNCTION__ << " Error: JSON cannot be empty.");
        THROW( SqlException, "Error: Entity json cannot be empty." );
    }

    if (!data[ENTITY_NAME_FIELD].isString()) {
        TRACE(__FUNCTION__ << "() found that " << ENTITY_NAME_FIELD << " is not string.")
        THROW( SqlException, ENTITY_NAME_FIELD << " is not string" );
    }
    entityName = data[ENTITY_NAME_FIELD].asString();

    if (!data[KEY_FIELD].isInt()) {
        TRACE(__FUNCTION__ << "() found that " << KEY_FIELD << " is not int.")
        THROW( SqlException, KEY_FIELD << " is not int" );
    }
    key = data[KEY_FIELD].asInt();

    std::unordered_map<std::string, std::string> types = EntitiesStructureCache::getSingletonInstance()->getFieldsTypes(entityName);
    for (const auto& member : data.getMemberNames()) {
        if (member == ENTITY_NAME_FIELD) continue;
        if (member == KEY_FIELD && key < 0) continue;
        if (data[member].isNull()) {
            value.emplace(member, Json::nullValue);
        } else if (types[member] == INT_JSON_TYPE) {
            if (!data[member].isInt()) {
                TRACE(__FUNCTION__ << "() found that " << member << " is not int.")
                THROW( SqlException, member << " is not int." );
            }
            value.emplace(member, data[member]);
        } else if (types[member] == FLOAT_JSON_TYPE) {
            if (!data[member].isDouble() && !(data[member].isInt() && data[member].asInt() == 0)) {
                TRACE(__FUNCTION__ << "() found that " << member << " is not float.")
                THROW( SqlException, member << " is not float." );
            }
            value.emplace(member, data[member]);
        } else if (types[member] == STRING_JSON_TYPE) {
            if (!data[member].isString()) {
                TRACE(__FUNCTION__ << "() found that " << member << " is not string.")
                THROW( SqlException, member << " is not string." );
            }
            value.emplace(member, data[member]);
        } else if (types[member] == BOOLEAN_JSON_TYPE) {
            if (!data[member].isBool()) {
                TRACE(__FUNCTION__ << "() found that " << member << " is not boolean.")
                THROW( SqlException, member << " is not boolean." );
            }
            value.emplace(member, data[member]);
        } else if (types[member] == BINARY_JSON_TYPE) {
            if (!data[member].isString()) {
                TRACE(__FUNCTION__ << "() found that " << member << " is not binary string.")
                THROW( SqlException, member << " is not binary string." );
            }
            value.emplace(member, data[member]);
        } else if (types[member] == DATE_JSON_TYPE) {
            if (!data[member].isString()) {
                TRACE(__FUNCTION__ << "() found that " << member << " is not date string.")
                THROW( SqlException, member << " is not date string." );
            }
            value.emplace(member, data[member]);
        } else if (types[member] == DATETIME_JSON_TYPE) {
            if (!data[member].isString()) {
                TRACE(__FUNCTION__ << "() found that " << member << " is not datetime string.")
                THROW( SqlException, member << " is not datetime string." );
            }
            value.emplace(member, data[member]);
        } else if (EntitiesStructureCache::getSingletonInstance()->isTypeEnum(types[member])) {
            std::vector<std::string> values = EntitiesStructureCache::getSingletonInstance()->getEnumValues(types[member]);
            if (!data[member].isString() || std::find(values.begin(), values.end(), data[member].asString()) == values.end()) {
                TRACE(__FUNCTION__ << "() found that " << member << " is not valid string.")
                THROW( SqlException, member << " is not valid string." );
            }
            value.emplace(member, data[member]);
        }
    }
    TRACE(__FUNCTION__ << " end point.");
}

bool JsonEntity::operator==( const JsonEntity &x ) const {
    if (this->entityName != x.entityName || this->key != x.key || value.size() != x.value.size()) {
        return false;
    }

    for (const auto& feature : value) {
        auto found = x.value.find(feature.first);
        if (found == x.value.end() || feature.second != found->second) {
            return false;
        }
    }

    return true;
}

bool JsonEntity::operator!=(const JsonEntity &x ) const {
    return !(*this==x);
}


Json::Value JsonEntity::toJson() const {
    Json::Value result;
    result[ENTITY_NAME_FIELD] = entityName;
    result[KEY_FIELD] = key;
    for (const auto& field : value) {
        result[field.first] = field.second;
    }
    return result;
}
