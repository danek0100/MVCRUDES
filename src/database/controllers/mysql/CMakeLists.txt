list(APPEND CORE_LIB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/MySqlCrudController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MySqlDatabaseController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MySqlSchemeLoader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MySqlSchemeUpdater.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/MySqlTypesConverter.cpp
)

set(CORE_LIB_SOURCES ${CORE_LIB_SOURCES} PARENT_SCOPE)
