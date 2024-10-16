#include    <include/database/controllers/postgresql/PostgreSqlTypesConverter.h>

PostgreSqlTypesConverter::PostgreSqlTypesConverter() : entitiesStructures(EntitiesStructureCache::getSingletonInstance()) {}

PostgreSqlTypesConverter::PostgreSqlTypesConverter(std::shared_ptr<EntitiesStructureCache> entitiesStructures)
                                                                      : entitiesStructures(std::move(entitiesStructures)) {}

PostgreSqlTypesConverter::~PostgreSqlTypesConverter() = default;

std::string PostgreSqlTypesConverter::getMappedType(const std::string& typeName, int length) {
    static const std::unordered_map<std::string, std::string> fixedTypeMappings = {
            {INT_JSON_TYPE, "int"},
            {FLOAT_JSON_TYPE, "double precision"},
            {BOOLEAN_JSON_TYPE, "mysql_bool"},
            {BINARY_JSON_TYPE, "bytea"},
            {DATE_JSON_TYPE, "date"},
            {DATETIME_JSON_TYPE, "timestamp without time zone"} // etc
    };

    auto it = fixedTypeMappings.find(typeName);
    if (it != fixedTypeMappings.end()) {
        return it->second;
    } else if (typeName == STRING_JSON_TYPE) {
        return "varchar(" + std::to_string(length > 0 ? length : DEFAULT_LENGTH) + ")";
    } else if (entitiesStructures->isTypeEnum(typeName)) {
        return typeName;
    } else {
        TRACE(__FUNCTION__ << "(). Unknown type: " << typeName)
        return "text"; // Безопасное дефолтное значение
    }
}

std::string PostgreSqlTypesConverter::getUnmappedType(const std::string& typeString) {
    static const std::unordered_map<std::string, std::string> fixedTypeMappings = {
            {"integer", INT_JSON_TYPE},
            {"double precision", FLOAT_JSON_TYPE},
            {"character varying", STRING_JSON_TYPE},
            {"text", STRING_JSON_TYPE},
            {"smallint", BOOLEAN_JSON_TYPE},
            {"bytea", BINARY_JSON_TYPE},
            {"date", DATE_JSON_TYPE},
            {"timestamp without time zone", DATETIME_JSON_TYPE} // etc
    };

    auto fixedTypeIt = fixedTypeMappings.find(typeString);
    if (fixedTypeIt != fixedTypeMappings.end()) {
        return fixedTypeIt->second;
    } else if (entitiesStructures->isTypeEnum(typeString)) {
        return typeString;
    }

    TRACE(__FUNCTION__ << "(). Unknown type: " << typeString)
    return "";
}

std::string PostgreSqlTypesConverter::adaptValueToType(const std::string& typeName, const Json::Value& value) {
    if (typeName == INT_JSON_TYPE) {
        return value.isNull() ? "NULL::" + getMappedType(INT_JSON_TYPE, 0) : std::to_string(value.asInt());
    } else if (typeName == FLOAT_JSON_TYPE) {
        return value.isNull() ? "NULL::" + getMappedType(FLOAT_JSON_TYPE, 0) : std::to_string(value.asDouble());
    } else if (typeName == STRING_JSON_TYPE) {
        return value.isNull() ? "NULL::CHARACTER VARYING" : "'" + StringTools::escapeSql(value.asString()) + "'";
    } else if (typeName == BINARY_JSON_TYPE) {
        return value.isNull() ? "NULL::" + getMappedType(BINARY_JSON_TYPE, 0) : "decode('" + value.asString() + "','hex')";
    } else if (typeName == BOOLEAN_JSON_TYPE) {
        return value.isNull() ? "NULL::" + getMappedType(BOOLEAN_JSON_TYPE, 0) : std::to_string((int) value.asBool());
    } else if (entitiesStructures->isTypeEnum(typeName)) {
        return value.isNull() ? "NULL::" + getMappedType(typeName, 0) : "'" + value.asString() + "'" + "::" + typeName; // преобразование text в enum-тип
    } else if (typeName == DATE_JSON_TYPE) {
        if (value.isNull()) {
            return "NULL::" + getMappedType(DATE_JSON_TYPE, 0);
        } else if (value.asString() == "Now") {
            return "now()";
        } else {
            return "'" + Date(value.asString()).get() + "'";
        }
    } else if (typeName == DATETIME_JSON_TYPE) {
        if (value.isNull()) {
            return "NULL::" + getMappedType(DATETIME_JSON_TYPE, 0);
        } else if (value.asString() == "Now") {
            return "now()";
        } else {
            return "'" + Datetime(value.asString()).get() + "'::timestamp without time zone";
        }
    }

    return std::to_string(value.asInt());
}
