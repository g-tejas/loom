cmake_minimum_required(VERSION 3.9)
project(loom)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# Quill
FetchContent_Declare(
        quill
        GIT_REPOSITORY https://github.com/odygrd/quill.git
        GIT_TAG master
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(quill)

# sockpp
FetchContent_Declare(
        sockpp
        GIT_REPOSITORY https://github.com/fpagliughi/sockpp
        GIT_TAG master
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(sockpp)
message(STATUS "sockpp include dir: ${sockpp_SOURCE_DIR}/include")

option(USE_EPOLL "" OFF)
option(USE_KQUEUE "" ON)

if ((NOT USE_EPOLL AND NOT USE_KQUEUE) OR (USE_EPOLL AND USE_KQUEUE))
    message(FATAL_ERROR "Please select one backend: epoll or kqueue")
endif ()


# add the library
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE include ${Boost_INCLUDE_DIRS})
if (USE_EPOLL)
    message(STATUS "LOOM: Using epoll backend")
    target_compile_definitions(${PROJECT_NAME} INTERFACE LOOM_BACKEND_EPOLL)
elseif (USE_KQUEUE)
    message(STATUS "LOOM: Using kqueue backend")
    target_compile_definitions(${PROJECT_NAME} INTERFACE LOOM_BACKEND_KQUEUE)
endif ()

target_link_libraries(${PROJECT_NAME} INTERFACE ${Boost_LIBRARIES} Tracy::TracyClient quill::quill sockpp)
