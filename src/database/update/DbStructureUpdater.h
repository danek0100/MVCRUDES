#ifndef SPHINXD_DBSTRUCTUREUPDATER_H
#define SPHINXD_DBSTRUCTUREUPDATER_H

#include    "./database/reader/SchemeReader.h"
#include    <experimental/filesystem>
#include    <utils/Singleton.h>

namespace fs = std::experimental::filesystem;

/**
 * @class DbStructureUpdater
 *
 * @brief Manages the update process of database structures based on defined schemas.
 *
 * DbStructureUpdater is responsible for ensuring the database structure matches the expected schema
 * definitions. It can apply updates to existing tables, create new tables, and manage version-controlled
 * custom updates to the database schema. The updater works by reading schema definitions from JSON files
 * and applying necessary changes to the database. It supports both automatic structural updates and
 * manual updates for specific versions.
 */
class DbStructureUpdater : public Singleton<DbStructureUpdater> {
private:
    DbStructureUpdater();
    void update(const std::string& path);
    std::list<std::string> addedEntities;
public:
    friend class Singleton<DbStructureUpdater>;

    /**
     * Applies updates to database structures based on the current application schemas.
     * This method reads schema definitions from the default entities directory and applies
     * any necessary changes to the database.
     */
    void updateStructures();

    /**
     * Applies updates to database structures for a specific custom update version.
     * This is used for applying version-specific changes that are not part of the regular
     * schema updates.
     *
     * @param customUpdateVersion The version number of the custom update to be applied.
     */
    void updateStructuresForCustom(int customUpdateVersion);

    /**
     * Retrieves a list of entities that were added to the database as part of the update process.
     *
     * @return A list of names of the added entities.
     */
    std::list<std::string> getAddedEntities();
};


#endif //SPHINXD_DBSTRUCTUREUPDATER_H
