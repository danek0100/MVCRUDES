#ifndef SPHINXD_STRUCTUREFIELD_H
#define SPHINXD_STRUCTUREFIELD_H

#include    "./json/json.h"
#include    <string>
#include    <unordered_map>
#include    <vector>

/**
 * @class StructureField
 *
 * @brief Represents a collection of key-value pairs defining the features of a structure.
 *
 * This class provides an interface to manage a set of features, each identified by a unique key.
 * It allows for adding, retrieving, and listing the keys of these features.
 */
class StructureField {
private:
    // Container for storing feature key-value pairs.
    std::unordered_map<std::string, Json::Value> features;

public:
    /**
     * Initializes an empty StructureField object without any features.
     */
    StructureField();

    /**
     * Initializes a StructureField object with a predefined set of features.
     * Each feature is represented as a key-value pair, allowing for immediate
     * and convenient population of the object's features upon creation.
     *
     * @param list An initializer list of key-value pairs, where each key is a
     *             string identifying the feature, and the value is a Json::Value
     *             representing the feature's value.
     */
    StructureField(std::initializer_list<std::pair<std::string, Json::Value>> list);

    /**
     * Retrieves the value associated with a given key.
     *
     * @param key The key for which the value is to be retrieved.
     * @return The value associated with the key, or an empty string if key is not found.
     */
    Json::Value get(const std::string& key) const;

    /**
     * Adds or updates a key-value pair in the features collection.
     *
     * @param key The key of the feature.
     * @param value The value of the feature.
     */
    void put(const std::string& key, const Json::Value& value);

    /**
     * Retrieves a list of all keys in the features collection.
     *
     * @return A vector containing all the keys of the features.
     */
    std::vector<std::string> getKeys() const;

    /**
     * Compares two StructureField objects for equality.
     * Two StructureField objects are considered equal if they contain the same set of key-value pairs.
     *
     * @param other The StructureField object to compare with.
     * @return True if the objects are equal, false otherwise.
     */
    bool operator==(const StructureField& other) const;
    bool operator!=(const StructureField& other) const;

    friend std::ostream &operator<<(std::ostream &stream, const StructureField &field);
};

#endif //SPHINXD_STRUCTUREFIELD_H
