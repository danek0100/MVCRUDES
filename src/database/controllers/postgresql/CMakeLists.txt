list(APPEND CORE_LIB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/PostgreSqlCrudController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PostgreSqlDatabaseController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PostgreSqlSchemeLoader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PostgreSqlSchemeUpdater.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/PostgreSqlTypesConverter.cpp
)

set(CORE_LIB_SOURCES ${CORE_LIB_SOURCES} PARENT_SCOPE)
