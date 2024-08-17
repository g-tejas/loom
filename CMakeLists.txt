cmake_minimum_required(VERSION 3.29)
project(loom)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
# Require dynamic_lookup for hooking system calls
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-undefined,dynamic_lookup")

option(LOOM_BUILD_TESTS "Build loom tests" ON)
option(LOOM_BUILD_EXAMPLES "Build loom examples" ON)
option(LOOM_BUILD_BENCHMARKS "Build loom benchmarks" ON)

add_subdirectory(third_party)

file(GLOB LOOM_SRC
		io/engine.cpp
        fiber/fiber.cpp)

if (APPLE)
	list(APPEND LOOM_SRC io/kqueue.cpp)
elseif (UNIX)
	list(APPEND LOOM_SRC io/epoll.cpp)
endif ()

message(STATUS "${PROJECT_NAME} Building library")
add_library(${PROJECT_NAME} SHARED ${LOOM_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC dl loom_third_party)

if (LOOM_BUILD_EXAMPLES)
    message(STATUS "${CMAKE_PROJECT_NAME}: Building examples")
    add_subdirectory(examples)
endif ()

if (LOOM_BUILD_TESTS)
    message(STATUS "${CMAKE_PROJECT_NAME}: Building tests")
    add_subdirectory(common/test)
    add_subdirectory(fiber/test)
endif ()

if (LOOM_BUILD_BENCHMARKS)
	message(STATUS "${CMAKE_PROJECT_NAME}: Building benchmarks")
	set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON)
	add_subdirectory(third_party/benchmark)
	add_subdirectory(benchmarks)
endif()
