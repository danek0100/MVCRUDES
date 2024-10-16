#include "EntitiesCacheActualizationTask.h"


EntitiesCacheActualizationTask * EntitiesCacheActualizationTask::get() {
    static EntitiesCacheActualizationTask instance;
    return &instance;
}

EntitiesCacheActualizationTask::EntitiesCacheActualizationTask(): RegularTask("EntitiesCacheActualizationTask") {
    TRACE(__FUNCTION__ << "().");
    setPeriod(5.0);
}

EntitiesCacheActualizationTask::~EntitiesCacheActualizationTask() = default;

bool EntitiesCacheActualizationTask::execute() {
    TRACE( __FUNCTION__ << "() entry point." );
    EntitiesCache::getSingletonInstance()->notifyNeedUpdate();
    TRACE( __FUNCTION__ << "(). Update request send. End point." );
    return true;
}
