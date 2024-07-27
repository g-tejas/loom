cmake_minimum_required(VERSION 3.29)
project(loom)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)

# Boost
find_package(Boost COMPONENTS context REQUIRED)
message(STATUS "Boost include dir: ${Boost_INCLUDE_DIRS}")

# Tracy
option(TRACY_ENABLE "" ON)
option(TRACY_FIBERS "" ON) # pdf page 38

FetchContent_Declare(
        tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG master
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(tracy)


# add the library
message(STATUS "${PROJECT_NAME} Building library")
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} INTERFACE ${Boost_LIBRARIES} Tracy::TracyClient)

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

if (BUILD_TESTS)
    add_subdirectory(tests)
endif ()
