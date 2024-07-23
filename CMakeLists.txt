cmake_minimum_required(VERSION 3.29)
project(loom)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)

add_subdirectory(lib)

if (BUILD_EXAMPLES)
    message(STATUS "LOOM: Building examples")
    add_subdirectory(examples)
endif ()

if (BUILD_TESTS)
    message(STATUS "LOOM: Building tests")
    add_subdirectory(tests)
endif ()
