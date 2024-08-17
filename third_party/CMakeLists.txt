add_subdirectory(fmt)

option(TRACY_ENABLE "" ON)
option(TRACY_FIBERS "" ON) # Refer to pdf page 38
add_subdirectory(tracy)

set(BUILD_TESTING OFF)
set(CATCH_INSTALL_DOCS OFF)
add_subdirectory(Catch2)

find_package(Boost COMPONENTS context REQUIRED)
message(STATUS "Boost include dir: ${Boost_INCLUDE_DIRS}")

add_library(loom_third_party INTERFACE)
target_include_directories(loom_third_party INTERFACE ${Boost_INCLUDE_DIRS})
target_link_libraries(loom_third_party INTERFACE fmt::fmt Tracy::TracyClient ${Boost_LIBRARIES})
