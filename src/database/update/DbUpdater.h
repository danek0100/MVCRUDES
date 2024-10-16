#ifndef SPHINXD_DBUPDATER_H
#define SPHINXD_DBUPDATER_H

#include    <utils/Singleton.h>

/**
 * @class DbUpdater
 *
 * @brief Manages database schema updates in a structured and version-controlled manner.
 *
 * DbUpdater provides mechanisms to apply custom and automatic updates to the database schema based on the current version
 * of the database. It ensures that the database schema is always in sync with the application's expectations and requirements.
 * Custom updates allow for specific schema modifications that cannot be handled automatically, while automatic updates apply
 * predefined changes that evolve with the application. DbUpdater operates as a Singleton to maintain a single point of control
 * and coordination for all database updates throughout the application.
 *
 * Usage:
 * DbUpdater::getSingletonInstance()->update(currentVersion);
 *
 * Where `currentVersion` is the current version of the database schema. The updater will apply all necessary updates
 * to reach the latest version, defined by ACTUAL_CUSTOM_VERSION, and then perform any additional automatic updates required.
 */
class DbUpdater : public Singleton<DbUpdater> {
private:
    DbUpdater();
    void makeCustomUpdates(int currentVersion);
    void makeAutoUpdate();

public:
    static const int ACTUAL_CUSTOM_VERSION = 373;

    friend class Singleton<DbUpdater>;
    int update(int currentVersion);
};


#endif //SPHINXD_DBUPDATER_H
