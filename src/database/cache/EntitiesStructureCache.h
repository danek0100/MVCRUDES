#ifndef SPHINXD_ENTITIESSTRUCTURECACHE_H
#define SPHINXD_ENTITIESSTRUCTURECACHE_H

#include    "./database/JsonsConstans.h"
#include    "./database/reader/EntityScheme.h"
#include    "./database/reader/StructureField.h"
#include    "./utils/Singleton.h"
#include    "./json/json.h"
#include    <include/database/cache/IEntitiesStructureCache.h>
#include    <Config.h>
#include    <experimental/filesystem>
#include    <fstream>
#include    <string>
#include    <vector>
#include    <unordered_map>


/**
 * @class EntitiesStructureCache
 *
 * @brief Caches the structure and properties of entities for efficient access and retrieval.
 *
 * This class is implemented as a singleton and manages caches for various aspects of entity structures.
 * It loads the entity structure and properties from JSON files in a specified directory and provides
 * methods to access this cached information. The cache includes details like field types, whether a provider
 * is needed for the entity, and additional properties of each field within the entities.
 *
 * Key functionalities:
 * - Load entity structures and properties from JSON files.
 * - Provide access to entity structures, types of fields, and other properties.
 * - Check if a specific entity requires a provider.
 * - List all keys (names) of cached entities.
 */
class EntitiesStructureCache: public IEntitiesStructureCache, public Singleton<EntitiesStructureCache> {
private:
    // Caches the enum types values.
    // type name -> possible values
    std::unordered_map<std::string, std::vector<std::string>> enumTypesValuesCache;

    // Caches the structure fields of table entities.
    std::unordered_map<std::string, std::vector<StructureField>> fieldsCache;

    // Caches the types of fields for each entity.
    // entity name -> field name -> name of type
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> typesCache;

    // Caches the provider needed.
    std::unordered_map<std::string, bool> providerCache;

    // Caches the entity versions.
    std::unordered_map<std::string, int> versionsCache;

    std::string loadedPath;

    // Constructor is private to enforce singleton pattern.
    EntitiesStructureCache();

    void clear();
public:
    // Grant Singleton class access to the private constructor.
    friend class Singleton<EntitiesStructureCache>;

    /**
     * Calculates whether type is enum and there are predefined values.
     *
     * @param key The identifier of the enum entity.
     * @return true, if type is enum, false otherwise.
     */
    bool isTypeEnum(const std::string& key) override;

    /**
     * Retrieves the possible values of an enum type.
     *
     * @param key The identifier of the enum entity.
     * @return A vector of strings representing enum value names.
     */
    std::vector<std::string> getEnumValues(const std::string& key) override;

    /**
     * Retrieves the structure of an entity.
     *
     * @param key The identifier of the entity.
     * @return A vector of StructureField representing the entity's structure.
     */
    std::vector<StructureField> getFields(const std::string& key);

    /**
     * Retrieves the types of fields for a specific entity.
     *
     * @param key The identifier of the entity.
     * @return A map of field names to their types.
     */
    std::unordered_map<std::string, std::string> getFieldsTypes(const std::string& key);

    /**
     * Retrieves the keys of all cached entities.
     *
     * @return A vector of strings representing the keys of all entities in the cache.
     */
    std::vector<std::string> getKeys();

    /**
     * Determines whether a provider is needed for a specific entity.
     *
     * This method checks if the given entity requires a provider based on the
     * current cache state. It is used to ensure that entities are properly
     * managed and updated as needed.
     *
     * @param entityName The name of the entity to check.
     * @return true if a provider is needed, false otherwise.
     */
    bool isNeedProvider(const std::string& entityName);

    /**
     * Retrieves the version of cached entity.
     *
     * @param entityName The name of the entity.
     * @return version.
     */
    int getVersion(const std::string& entityName);

    // Loads cache data from a path.
    void loadCache(const std::string& directoryPath);

};

#endif //SPHINXD_ENTITIESSTRUCTURECACHE_H
