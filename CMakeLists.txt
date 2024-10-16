cmake_minimum_required(VERSION 3.15)

project(mvcrudes C CXX)

set(CMAKE_C_FLAGS "" CACHE STRING "c flags" FORCE)
set(CMAKE_C_FLAGS_DEBUG "" CACHE STRING "c flags debug" FORCE)
set(CMAKE_C_FLAGS_RELEASE "" CACHE STRING "c flags release" FORCE)
#
set(CMAKE_CXX_FLAGS "" CACHE STRING "cxx flags" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "" CACHE STRING "cxx flags debug" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE  "" CACHE STRING "cxx flags release" FORCE)
#
set(CMAKE_EXE_LINKER_FLAGS "" CACHE STRING "exe linker flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "" CACHE STRING "exe linker flags debug" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "" CACHE STRING "exe linker flags release" FORCE)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME})

if("Linux" STREQUAL "${CMAKE_SYSTEM_NAME}")
    message(FATAL_ERROR "not implemented yet")
elseif("Windows" STREQUAL "${CMAKE_SYSTEM_NAME}")
    message(FATAL_ERROR "not implemented yet")
    if(CMAKE_GENERATOR_PLATFORM STREQUAL "Win32")
        include(sphinxd_win32.cmake)
    else()
        include(sphinxd_win64.cmake)
    endif()
elseif("Darwin" STREQUAL "${CMAKE_SYSTEM_NAME}")
    include(sphinxd_win32_darwin.cmake)
endif()

target_sources(
    ${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Main.cpp
)

add_dependencies(${PROJECT_NAME} update-build-time)

set(CORE_LIB_SOURCES)

set(BUILD_GMOCK ON)
add_subdirectory(lib_googletest)

add_subdirectory(database)

option(CLASSIC_BUILD "Old version of single sphinxd build" OFF)

# Variable for test sources
set(TEST_SOURCES "")
enable_testing()

add_subdirectory(tests)

target_include_directories(
    ${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/lib_googletest/googletest/include
    ${CMAKE_SOURCE_DIR}/lib_googletest/googletest
    ${CMAKE_SOURCE_DIR}/lib_googletest/googlemock
    ${CMAKE_SOURCE_DIR}/lib_googletest/googlemock/include
)


if(${CLASSIC_BUILD})
    message("Classic build target without tests")

    target_sources(
        ${PROJECT_NAME} PRIVATE
        ${CORE_LIB_SOURCES}
    )
else()
    message("Standard build with tests")

    target_sources(
        ${PROJECT_NAME} PRIVATE
        ${CORE_LIB_SOURCES}
        ${TEST_SOURCES}
    )

    target_link_libraries(
        ${PROJECT_NAME} PRIVATE
        gtest
        gmock
    )
endif()
