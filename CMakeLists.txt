cmake_minimum_required(VERSION 3.13)

# Define the project (Crucial for CMake to find compilers)
project(AutoDecoration)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(Geode)

# Create the mod
setup_geode_mod(mircels.auto_decoration)

# Add sources directly (fixing the recursion error)
target_sources(${PROJECT_NAME} PRIVATE main.cpp)

# Link binaries
add_geode_binaries(mircels.auto_decoration)
