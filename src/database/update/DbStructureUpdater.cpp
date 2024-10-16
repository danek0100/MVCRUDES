#include    "DbStructureUpdater.h"
#include    "./database/manager/DatabaseManager.h"

DbStructureUpdater::DbStructureUpdater() : Singleton<DbStructureUpdater>() {}

void DbStructureUpdater::update(const std::string& path) {
    auto& dbController = DatabaseManager::getDatabaseController();
    auto& schemeLoader = DatabaseManager::getSchemeLoader();
    auto& schemeUpdater = DatabaseManager::getSchemeUpdater();
    std::list<std::shared_ptr<EntityScheme>> requiredSchemes = SchemeReader::getSingletonInstance()->readSchemes(path);

    for (const auto& scheme : requiredSchemes) {
        int currentVersion = dbController->getEntityVersion(scheme->name);
        if (scheme->version != -1) {
            // Table doesn't exists
            if (currentVersion == -2) {
                if (const auto* tableScheme = dynamic_cast<const TableScheme*>(scheme.get())) {
                    if (DatabaseManager::getSingletonInstance()->initTable(*tableScheme) != 0) {
                        THROW(SystemException, "Cannot create table: " << scheme->name)
                    };
                }
                addedEntities.push_back(scheme->name);
            }
            else if (scheme->version > currentVersion) {
                TableScheme currentScheme = schemeLoader->getActualScheme(scheme->name);
                if (const auto* tableScheme = dynamic_cast<const TableScheme*>(scheme.get())) {
                    if (schemeUpdater->updateCurrentScheme(currentScheme, *tableScheme) != 0) {
                        THROW(SystemException, "Cannot update table: " << scheme->name)
                    }
                }
            }
        }
    }
}

void DbStructureUpdater::updateStructures() {
    std::string path = SchemeReader::getSingletonInstance()->getDefaultJsonsDirectoryPath() + "/entities";
    EntitiesStructureCache::getSingletonInstance()->loadCache(path);
    update(path);
}

void DbStructureUpdater::updateStructuresForCustom(int customUpdateVersion) {
    std::string path = SchemeReader::getSingletonInstance()->getDefaultJsonsDirectoryPath() + ("/frozen/" + std::to_string(customUpdateVersion));
    EntitiesStructureCache::getSingletonInstance()->loadCache(path);
    update(path);
}

std::list<std::string> DbStructureUpdater::getAddedEntities() {
    return addedEntities;
}
