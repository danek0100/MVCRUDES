#include    <include/database/controllers/mysql/MySqlTypesConverter.h>

#include <utility>

MySqlTypesConverter::MySqlTypesConverter() : entitiesStructures(EntitiesStructureCache::getSingletonInstance()) {}

MySqlTypesConverter::MySqlTypesConverter(std::shared_ptr<EntitiesStructureCache> entitiesStructures)
                                                                       : entitiesStructures(std::move(entitiesStructures)) {}

MySqlTypesConverter::~MySqlTypesConverter() = default;

std::string MySqlTypesConverter::adaptValueToType(const std::string& typeName, const Json::Value& value) {
    if (value.isNull()) {
        return "NULL";
    }

    if (typeName == INT_JSON_TYPE) {
        return std::to_string(value.asInt());
    } else if (typeName == FLOAT_JSON_TYPE) {
        return std::to_string(value.asDouble());
    } else if (typeName == STRING_JSON_TYPE || entitiesStructures->isTypeEnum(typeName)) {
        return "'" + StringTools::escapeSql(value.asString()) + "'";
    } else if (typeName == BINARY_JSON_TYPE) {
        return "UNHEX('" + value.asString() + "')";
    } else if (typeName == BOOLEAN_JSON_TYPE) {
        return std::to_string((int) value.asBool());
    } else if (typeName == DATE_JSON_TYPE) {
        if (value.asString() == "Now") {
            return "current_timestamp()";
        } else {
            return "'" + Date(value.asString()).get() + "'";
        }
    } else if (typeName == DATETIME_JSON_TYPE) {
        if (value.asString() == "Now") {
            return "current_timestamp()";
        } else {
            return "'" + Datetime(value.asString()).get() + "'";
        }
    }

    return std::to_string(value.asInt());
}

std::string MySqlTypesConverter::getMappedType(const std::string& typeName, int length) {
    static const std::unordered_map<std::string, std::string> fixedTypeMappings = {
            {FLOAT_JSON_TYPE, "double"},
            {BOOLEAN_JSON_TYPE, "boolean"},
            {DATE_JSON_TYPE, "date"},
            {DATETIME_JSON_TYPE, "datetime"} // etc
    };

    auto it = fixedTypeMappings.find(typeName);
    if (it != fixedTypeMappings.end()) {
        return it->second;
    } else if (typeName == STRING_JSON_TYPE) {
        // MySQL имеет ограничение на длину всей строки (row): 65 536 байт.
        // Такое же ограничение наследуют и колонки (размер колонки не может быть больше размера строки).
        // UTF-8 кодировка отводит под символы 3 байта, создавая ограничение на колонку в размере 21 845 символов.
        // Подробнее об ограничениях размера: https://dev.mysql.com/doc/refman/8.4/en/column-count-limit.html.
        // Для преодоления ограничения размера используется mediumtext, т. к. он вносит в общий размер колонки 9-12 байт,
        // а содержимое хранит отдельно. Содержимое может достигать 16 мегабайт (5 592 405 символов).
        // Используется не text, т. к. у него то же ограничение на 65 536 байт (21 845 символов).
        if (length > 21845) return "mediumtext CHARACTER SET UTF8";
        else return "varchar(" + std::to_string(length > 0 ? length : DEFAULT_LENGTH) + ") CHARACTER SET UTF8";
    } else if (typeName == INT_JSON_TYPE) {
        if (length>11) return "bigint(" + std::to_string(length) + ")";
        else if(length>0)return "int(" + std::to_string(length) + ")";
        else return "int";
    } else if (typeName == BINARY_JSON_TYPE) {
        if (length>65535) return "longblob";
        else if (length>21845) return "blob";
        return "varbinary(" + std::to_string(length > 0 ? length : DEFAULT_LENGTH) + ")";
    } else if (entitiesStructures->isTypeEnum(typeName)) {
        std::vector<std::string> enumValues = entitiesStructures->getEnumValues(typeName);
        std::string enumValuesStr;
        for (const auto& enumValue : enumValues) {
            enumValuesStr += ("'" + enumValue + "',");
        }
        enumValuesStr.pop_back(); // remove trailing comma
        return "enum(" + enumValuesStr + ")";
    } else {
        TRACE(__FUNCTION__ << "(). Unknown type: " << typeName)
        return "text"; // Безопасное дефолтное значение
    }
}

std::string MySqlTypesConverter::getUnmappedType(const std::string& typeString) {
    static const std::unordered_map<std::string, std::string> fixedTypeMappings = {
            {"integer", INT_JSON_TYPE},
            {"double precision", FLOAT_JSON_TYPE},
            {"varchar", STRING_JSON_TYPE},
            {"text", STRING_JSON_TYPE},
            {"mediumtext", STRING_JSON_TYPE},
            {"boolean", BOOLEAN_JSON_TYPE},
            {"varbinary", BINARY_JSON_TYPE},
            {"tinyblob", BINARY_JSON_TYPE},
            {"blob", BINARY_JSON_TYPE},
            {"longblob", BINARY_JSON_TYPE},
            {"date", DATE_JSON_TYPE},
            {"datetime", DATETIME_JSON_TYPE} // etc
    };

    auto fixedTypeIt = fixedTypeMappings.find(typeString);
    if (fixedTypeIt != fixedTypeMappings.end()) {
        return fixedTypeIt->second;
    } else if (typeString.substr(0, 4) == "enum") {
        return typeString.substr(5, typeString.find_first_of(',') - 5); // from 'enum(' to first comma
    }

    TRACE(__FUNCTION__ << "(). Unknown type: " << typeString)
    return "";
}
