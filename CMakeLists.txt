cmake_minimum_required(VERSION 3.29)
project(eventloop)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# pass cmake configs to build tests or not by default
option(BUILD_BINARY "Build binary" ON)
option(BUILD_TESTS "Build tests" ON)

add_subdirectory(external)

set(SOURCES
        src/main.cpp
        src/thread.cpp
        src/reactor.cpp
        src/backends/darwin.cpp)

if (BUILD_BINARY)
    message(STATUS "Building binary")
    include_directories(include)
    add_executable(${PROJECT_NAME} ${SOURCES})
    target_include_directories(${PROJECT_NAME} PUBLIC include)
    target_link_libraries(${PROJECT_NAME} PUBLIC external)
endif ()

if (BUILD_TESTS)
    message(STATUS "Building tests")
    add_subdirectory(tests)
endif ()
