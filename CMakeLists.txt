cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME "AutoBuild")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

# Access environment variable directly
if (DEFINED ENV{GEODE_SDK})
    file(TO_CMAKE_PATH "$ENV{GEODE_SDK}" GEODE_SDK_PATH)
    message(STATUS "Found GEODE_SDK in env: ${GEODE_SDK_PATH}")
elseif (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/geode")
    set(GEODE_SDK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/geode")
    message(STATUS "Found geode in local directory: ${GEODE_SDK_PATH}")
else()
    message(FATAL_ERROR "Geode SDK not found in environment or local directory!")
endif()

# Add to search paths
list(APPEND CMAKE_MODULE_PATH "${GEODE_SDK_PATH}/cmake")
list(APPEND CMAKE_PREFIX_PATH "${GEODE_SDK_PATH}")

find_package(Geode REQUIRED)

set(SOURCE_FILES
    src/main.cpp
    src/AutoBuildPopup.cpp
    src/DecorationManager.cpp
)

setup_geode_mod(
    SOURCES ${SOURCE_FILES}
)
