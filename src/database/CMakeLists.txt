add_subdirectory(reader)
add_subdirectory(cache)
add_subdirectory(manager)
add_subdirectory(controllers)
add_subdirectory(provider)
add_subdirectory(update)

set(CORE_LIB_SOURCES ${CORE_LIB_SOURCES} PARENT_SCOPE)
