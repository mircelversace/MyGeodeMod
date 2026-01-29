cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(AutoDecoration VERSION 1.0.0)

# Find Geode SDK via environment variable
if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Define GEODE_SDK environment variable to point to Geode SDK")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# Setup the mod
setup_geode_mod(${PROJECT_NAME})
