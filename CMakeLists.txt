cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME "AutoBuild")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT DEFINED GEODE_SDK)
	if (DEFINED ENV{GEODE_SDK})
		set(GEODE_SDK $ENV{GEODE_SDK})
	elseif (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/geode)
		set(GEODE_SDK ${CMAKE_CURRENT_SOURCE_DIR}/geode)
	else()
		message(FATAL_ERROR "Unable to find Geode SDK! Please clone geode-sdk/geode into your project directory or set the GEODE_SDK environment variable.")
	endif()
endif()

list(APPEND CMAKE_MODULE_PATH ${GEODE_SDK}/cmake)

find_package(Geode REQUIRED)

set(SOURCE_FILES
    src/main.cpp
    src/AutoBuildPopup.cpp
    src/DecorationManager.cpp
)

setup_geode_mod(
    SOURCES ${SOURCE_FILES}
)
