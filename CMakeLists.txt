cmake_minimum_required(VERSION 3.16)
project(MonteCarloVolume LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/main.cpp
)

add_executable(MonteCarloVolume ${SOURCES})

target_include_directories(MonteCarloVolume PRIVATE ${PROJECT_SOURCE_DIR}/include)

option(USE_OpenMP "Use OpenMP" ON)
if(USE_OpenMP)
    find_package(OpenMP)
    if(OpenMP_CXX_FOUND)
        message(STATUS "OpenMP found")
        target_link_libraries(MonteCarloVolume PRIVATE OpenMP::OpenMP_CXX)
    else()
        message(WARNING "OpenMP requested but not found")
    endif()
endif()
