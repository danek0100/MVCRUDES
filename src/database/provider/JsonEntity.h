#ifndef SPHINXD_JSONENTITY_H
#define SPHINXD_JSONENTITY_H

#include    "./database/JsonsConstans.h"
#include    "./database/cache/EntitiesStructureCache.h"
#include    <json/value.h>
#include    <string>
#include    <sql/Sql.h>
#include    <utility>
#include    <unordered_map>

/**
 * @class JsonEntity
 *
 * @brief Represents an entity in JSON format.
 *
 * This class encapsulates an entity with a name and its corresponding values. It provides
 * functionality to convert to and from a JSON format, along with comparison operators for
 * evaluating equality between entities.
 */
class JsonEntity {
public:
    std::string entityName;
    int key;
    std::unordered_map<std::string, Json::Value> value;

    /**
     * Constructor for JsonEntity with name and value map.
     *
     * @param _entityName Name of the entity.
     * @param _key Key of the entity.
     * @param _value Map of attribute names to their values.
     */
    JsonEntity(std::string _entityName, int _key, std::unordered_map<std::string, Json::Value> _value);

    /**
     * Constructor for JsonEntity from a JSON value.
     *
     * @param data JSON value representing the entity.
     */
    JsonEntity(const Json::Value& data);

    /**
     * Equality operator.
     *
     * @param x JsonEntity to compare with.
     * @return true if both entities are equal, false otherwise.
     */
    bool operator==(const JsonEntity &x) const;

    /**
     * Inequality operator.
     *
     * @param x JsonEntity to compare with.
     * @return true if entities are not equal, false otherwise.
     */
    bool operator!=(const JsonEntity &x) const;

    /**
     * Converts the entity to a JSON value.
     *
     * @return JSON representation of the entity.
     */
    Json::Value toJson() const;
};

#endif //SPHINXD_JSONENTITY_H
