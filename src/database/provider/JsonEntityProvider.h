#ifndef SPHINXD_JSONENTITYPROVIDER_H
#define SPHINXD_JSONENTITYPROVIDER_H

#include    "JsonEntity.h"
#include    "JsonProvider.h"
#include    "JsonProviderWithFilteredSpeaker.h"
#include    "JsonEntityFilter.h"
#include    "./database/JsonsConstans.h"
#include    "./database/cache/EntitiesStructureCache.h"
#include    "./database/provider/JsonEntityFilter.h"
#include    "./database/reader/StructureField.h"
#include    "./database/manager/DatabaseManager.h"
#include    <DbUsersConfig.h>
#include    <list>
#include    <string>
#include    <utility>
#include    <utils/Singleton.h>
#include    <fw/common/StringTools.h>


/**
 * @class JsonEntityProvider
 * @brief Provides CRUD operations and change notifications for JsonEntity objects.
 *
 * Inherits from Singleton<JsonEntityProvider> to ensure a single instance and
 * JsonProviderWithFilteredSpeaker<JsonEntity> for event handling. It manages
 * CRUD operations for JsonEntity objects and notifies subscribers about changes.
 */
class JsonEntityProvider : public Singleton<JsonEntityProvider>, public JsonProviderWithFilteredSpeaker<JsonEntity> {
private:
    std::unique_ptr<ICrudController> database;

    JsonEntityProvider();

    // Implementation of CRUD operations.
    void createImpl(std::list<JsonEntity>& object) override;
    std::list<JsonEntity> readImpl(const std::string& entityName, const Json::Value& filter, const std::set<std::string>& fields = {}, bool forceRead = false) override;
    std::list<JsonEntity> updateImpl(std::list<JsonEntity>& object) override;
    std::list<JsonEntity> removeImpl(std::list<JsonEntity>& objects) override;

    // Fill entity in place
    void fillEntity(JsonEntity& object);
    // Check entity and trow exceptions in place
    void checkEntity(const JsonEntity& object);
    // Check fields of object on unique
    void checkUniqueFields(const JsonEntity &object, const std::vector<std::string>& fields);

public:
    friend Singleton;
    void notifyProviderAboutChanges(uint32_t event, const JsonEntity& object);
};

#endif //SPHINXD_JSONENTITYPROVIDER_H
