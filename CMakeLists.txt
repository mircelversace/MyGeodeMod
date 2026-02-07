cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME "AutoBuild")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Geode REQUIRED)

set(SOURCE_FILES
    src/main.cpp
    src/AutoBuildPopup.cpp
    src/DecorationManager.cpp
)

setup_geode_mod(
    SOURCES ${SOURCE_FILES}
)
