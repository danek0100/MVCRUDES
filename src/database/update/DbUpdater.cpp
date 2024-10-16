#include    "DbStructureUpdater.h"
#include    "DbUpdater.h"
#include    "./database/manager/DatabaseManager.h"
#include    "./database/update/custom/CustomUpdateFactory.h"

/**
 * Ќ≈ ”ƒјЋя“№!
 * Ёто механизм загрузки custom-обновлений Ѕƒ, которые никак иначе не объ€вить.
 */
#include    "./custom/CustomUpdateRegistry.h"

DbUpdater::DbUpdater() : Singleton<DbUpdater>() {}

int DbUpdater::update(int currentVersion) {
    try {
        if (currentVersion < ACTUAL_CUSTOM_VERSION) {
            makeCustomUpdates(currentVersion);
        }
        makeAutoUpdate();
    } catch (Exception* ex) {
        TRACE("Update error: " << ex->getText());
        delete ex;
        return -1;
    }
    return 0;
}

void DbUpdater::makeCustomUpdates(int currentVersion) {
    for (int version = currentVersion + 1; version <= ACTUAL_CUSTOM_VERSION; ++version) {
        std::unique_ptr<CustomUpdate> update(CustomUpdateFactory::createUpdate(std::to_string(version)));
        if (update) {
            update->update();
        }
    }
}

void DbUpdater::makeAutoUpdate() {
    DbStructureUpdater::getSingletonInstance()->updateStructures();

    // TODO process result code if needed
    DatabaseManager::getSingletonInstance()->addDefaults(
            DbStructureUpdater::getSingletonInstance()->getAddedEntities()
            );
}
