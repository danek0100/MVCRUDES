#ifndef SPHINXD_ENTITIESCACHEACTUALIZATIONTASK_H
#define SPHINXD_ENTITIESCACHEACTUALIZATIONTASK_H

#include    "EntitiesCache.h"
#include    "./TaskManager/RegularTask.h"

/**
 * EntitiesCacheActualizationTask is a specialized task that periodically actualizes the EntitiesCache.
 * It inherits from RegularTask, providing it with the capability to be scheduled and executed at regular intervals
 * by the TaskManager. The primary function of this task is to trigger the update mechanism within the EntitiesCache,
 * ensuring that the cache is refreshed with the latest data from the database. This class employs the Singleton
 * pattern to ensure that only one instance of the task is created and used throughout the application, maintaining
 * efficient and coherent cache update operations. The execute method is overridden to perform the actual cache
 * actualization logic, making it a critical component for maintaining data consistency and timeliness in
 * applications relying on cached data.
 */
class EntitiesCacheActualizationTask : public RegularTask {
private:
    EntitiesCacheActualizationTask();
    ~EntitiesCacheActualizationTask() override;

public:
    static EntitiesCacheActualizationTask * get();
    bool execute() override;
};

#endif //SPHINXD_ENTITIESCACHEACTUALIZATIONTASK_H
