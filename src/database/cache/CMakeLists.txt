list(APPEND CORE_LIB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/EntitiesStructureCache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/EntitiesCache.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/EntitiesCacheActualizationTask.cpp
)

set(CORE_LIB_SOURCES ${CORE_LIB_SOURCES} PARENT_SCOPE)
