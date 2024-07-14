cmake_minimum_required(VERSION 3.29)
project(flux)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)

add_subdirectory(libflux)

if (BUILD_EXAMPLES)
    message(STATUS "FLUX: Building examples")
    add_subdirectory(examples)
endif ()

if (BUILD_TESTS)
    message(STATUS "FLUX: Building tests")
    add_subdirectory(tests)
endif ()
