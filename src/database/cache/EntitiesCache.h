#ifndef SPHINXD_ENTITIESCACHE_H
#define SPHINXD_ENTITIESCACHE_H

#include    "EntitiesCache.h"
#include    "./database/manager/DatabaseManager.h"
#include    "./database/provider/JsonEntity.h"
#include    <utils/Changes.h>
#include    <utils/Singleton.h>
#include    <utils/SingleShotTaskDataUpdater.h>

/**
 * EntitiesCache is a caching layer that manages the storage and retrieval of entity data in a high-performance manner.
 * It inherits from SingleShotTaskDataUpdater for periodic updates based on database changes and Singleton to ensure
 * a single instance is used throughout the application. It uses an unordered_set to maintain a list of entity names
 * that are currently cached. The class provides functionality to load entity data from the database, add, remove,
 * and update entities in the cache. It also maintains data integrity and consistency across the application by
 * leveraging mutexes for thread-safe data access. Additionally, it implements a subscriber pattern to listen for
 * entity changes and update the cache accordingly. This class is an essential component for applications requiring
 * efficient and fast data access patterns.
 */
class EntitiesCache : public SingleShotTaskDataUpdater, public Singleton<EntitiesCache> {
private:
    std::unordered_set<std::string> cachedEntitiesNames = {"PARAMI", "PARAMB"};

    std::unique_ptr<ICrudController> database;
    std::unordered_map<std::string, std::unique_ptr<Mutex>> dataMutexes;
    std::unordered_map<std::string, std::list<JsonEntity>> data;

    EntitiesCache();

    /**
     * Loads the data for a specific entity from the database.
     *
     * @param entityName The name of the entity for which data is to be loaded.
     * @return A list of JsonEntity objects representing the loaded data.
     */
    std::list<JsonEntity> loadData(const std::string& entityName);

    /**
     * Periodically calculates and updates the internal data cache based on database changes.
     */
    void calculateData() override;

public:
    friend class Singleton<EntitiesCache>;
    bool hasCache(const std::string& entityName);

    // CRUD operations
    std::list<JsonEntity> getCache(const std::string& entityName);
    void addEntity(const JsonEntity& object);
    void removeEntity(const JsonEntity& object);
    void updateEntity(const JsonEntity& object);
};


#endif //SPHINXD_ENTITIESCACHE_H
