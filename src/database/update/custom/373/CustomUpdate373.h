#ifndef SPHINXD_CUSTOMUPDATE373_H
#define SPHINXD_CUSTOMUPDATE373_H

#include    "./database/manager/DatabaseManager.h"
#include    "./database/update/custom/CustomUpdate.h"
#include    "./database/update/custom/CustomUpdateFactory.h"
#include    "./database/update/DbStructureUpdater.h"
#include    <DbUsersConfig.h>

/**
 * @class CustomUpdate373
 *
 * @brief Implements the database update logic for version 373.
 *
 * This update prepares the server for further autonomous work with PARAMI/PARAMB tables by adjusting unique constraints
 * and updating specific entries. It supports different types of database servers by providing server-specific update queries.
 * Additionally, it updates the database version in the PARAMI table to reflect the application of this update.
 */
class CustomUpdate373 : public CustomUpdate {
public:
    int update() override;

private:
    void parseAccessRuleCond(Json::Value &pValue, SqlResult &sqlResult);
    void updatePayMenuItems();
};

REGISTER_CUSTOM_UPDATE("373", CustomUpdate373)

#endif //SPHINXD_CUSTOMUPDATE373_H
