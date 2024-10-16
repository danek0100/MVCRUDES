list(APPEND CORE_LIB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/DbUpdater.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/DbStructureUpdater.cpp
)

add_subdirectory(custom)

set(CORE_LIB_SOURCES ${CORE_LIB_SOURCES} PARENT_SCOPE)
