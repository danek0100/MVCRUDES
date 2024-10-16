list(APPEND CORE_LIB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/JsonEntity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/JsonEntityProvider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/JsonEntityFilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/JsonEntityGateProvider.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/JsonEntityListener.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/JsonEntitySubscriber.cpp
)

set(CORE_LIB_SOURCES ${CORE_LIB_SOURCES} PARENT_SCOPE)
