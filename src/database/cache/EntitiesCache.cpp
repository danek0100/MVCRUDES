#include    "EntitiesCache.h"
#include    "./database/provider/JsonEntityProvider.h"
#include    "./database/provider/JsonEntityListener.h"


EntitiesCache::EntitiesCache() :
    SingleShotTaskDataUpdater(), Singleton<EntitiesCache>(), database(DatabaseManager::getCrudController().get()) {

    TRACE( __FUNCTION__ << "() entry point. Data loading.");
    for (const auto& key : cachedEntitiesNames) {
        dataMutexes.emplace(std::make_pair(key, std::make_unique<Mutex>("JsonEntityProvider " + key + " mutex")));
        data[key] = loadData(key);
    }

    TRACE( __FUNCTION__ << "() Initial data loaded. Subscriber adding.");
    std::function<void(uint32_t, JsonEntity)> cacheCallback = [this](uint32_t event, const JsonEntity& object) {
        if (event == JsonEntityListener::EVENT_ADD) {
            addEntity(object);
        } else if (event == JsonEntityListener::EVENT_REMOVE) {
            removeEntity(object);
        } else if (event == JsonEntityListener::EVENT_UPDATE) {
            updateEntity(object);
        }
    };

    std::function<bool(JsonEntity)> cacheFilterFunc = [this](const JsonEntity& object) {
        if (hasCache(object.entityName)) {
            return true;
        } else {
            return false;
        }
    };
    JsonEntitySubscriber jsonEntityCacheSubscriber("JsonEntitiesCacheSubscriber", cacheCallback, cacheFilterFunc);
    JsonEntityListener::getSingletonInstance()->addSubscriber(jsonEntityCacheSubscriber);
    TRACE( __FUNCTION__ << "() Subscriber added");

    notifyNeedUpdate();
    TRACE( __FUNCTION__ << "() end point");
}

std::list<JsonEntity> EntitiesCache::loadData(const std::string& entityName) {
    TRACE(__FUNCTION__ << "() entry point")
    std::list<JsonEntity> result = database->loadEntities(entityName, {Json::ValueType::nullValue});
    TRACE(__FUNCTION__ << "() end point")
    return result;
}

bool EntitiesCache::hasCache(const std::string& entityName) {
    if (cachedEntitiesNames.find(entityName) != cachedEntitiesNames.end()) {
        return true;
    }
    return false;
}

std::list<JsonEntity> EntitiesCache::getCache(const std::string &entityName) {
    TRACE(__FUNCTION__ << "() " << entityName << " entry point")
    std::list<JsonEntity> result;
    TRACE(__FUNCTION__ << "() end point")
    MutexLock lock(dataMutexes.at(entityName).get());
    return hasCache(entityName) ? data.at(entityName) : result;
}

void EntitiesCache::addEntity(const JsonEntity& object) {
    TRACE(__FUNCTION__ << "() entry point")
    if (hasCache(object.entityName)) {
        MutexLock lock(dataMutexes.at(object.entityName).get());
        data[object.entityName].push_back(object);
    }
    TRACE(__FUNCTION__ << "() end point")
}

void EntitiesCache::removeEntity(const JsonEntity& object) {
    TRACE(__FUNCTION__ << "() entry point")
    if (hasCache(object.entityName)) {
        MutexLock lock(dataMutexes.at(object.entityName).get());
        for (auto it = data.at(object.entityName).begin(); it != data.at(object.entityName).end();) {
            if (it->key == object.key) {
                it = data.at(object.entityName).erase(it);
            } else {
                ++it;
            }
        }
    }
    TRACE(__FUNCTION__ << "() end point")
}

void EntitiesCache::updateEntity(const JsonEntity& object) {
    TRACE(__FUNCTION__ << "() entry point")
    if (hasCache(object.entityName)) {
        MutexLock lock(dataMutexes.at(object.entityName).get());
        for (auto it = data.at(object.entityName).begin(); it != data.at(object.entityName).end();) {
            if (it->key == object.key) {
                it = data.at(object.entityName).erase(it);
            } else {
                ++it;
            }
        }
        data.at(object.entityName).push_back(object);
    }
    TRACE(__FUNCTION__ << "() end point")
}

void EntitiesCache::calculateData() {
    TRACE(__FUNCTION__ << "() entry point")

    for (const auto& entityName : cachedEntitiesNames) {
        TRACE(__FUNCTION__ << "(). " << entityName << " check. Entry point.")
        std::list<JsonEntity> newData = loadData(entityName);
        std::list<JsonEntity> oldData = {newData};

        {
            MutexLock lock(dataMutexes.at(entityName).get());
            data.at(entityName).swap(oldData);
        }

        Changes<std::list<JsonEntity>> changes(
                oldData,
                newData,
                [](const JsonEntity &o1, const JsonEntity &o2)
                { return o1.key == o2.key; },
                [](const JsonEntity &o1, const JsonEntity &o2)
                { return o1 == o2; });

        for (const auto &item: changes.toAdd) {
            JsonEntityProvider::getSingletonInstance()->notifyProviderAboutChanges(JsonEntityProvider::EVENT_ADD, item);
        }
        for (const auto &item: changes.toChange) {
            JsonEntityProvider::getSingletonInstance()->notifyProviderAboutChanges(JsonEntityProvider::EVENT_UPDATE, item);
        }
        for (const auto &item: changes.toRemove) {
            JsonEntityProvider::getSingletonInstance()->notifyProviderAboutChanges(JsonEntityProvider::EVENT_REMOVE, item);
        }
        TRACE(__FUNCTION__ << "(). " << entityName << " check. End point.")
    }
    TRACE(__FUNCTION__ << "() end point")
}
