cmake_minimum_required(VERSION 3.29)
project(eventloop-tests)
Include(FetchContent)

# Catch2 for testing
set(BUILD_TESTING OFF)
set(CATCH_INSTALL_DOCS OFF)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.6.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(${PROJECT_NAME} fifo_test.cpp)
target_link_libraries(${PROJECT_NAME} Catch2::Catch2WithMain libflux)

# Register tests with CTest
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME})