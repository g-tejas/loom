cmake_minimum_required(VERSION 3.9)
project(libflux)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Boost
find_package(Boost COMPONENTS context REQUIRED)
message(STATUS "Boost include dir: ${Boost_INCLUDE_DIRS}")

# Tracy
set(TRACY_ENABLE ON)
set(TRACY_FIBERS ON) # pdf page 38
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


# specify the source files
set(SOURCES
        src/thread.cpp
        src/reactor.cpp
        src/backends/darwin.cpp)

# add the library
add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC include ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${Boost_LIBRARIES} Tracy::TracyClient quill::quill)