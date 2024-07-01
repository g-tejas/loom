find_package(Boost COMPONENTS context REQUIRED)
message(STATUS "Boost include dir: ${Boost_INCLUDE_DIRS}")

add_subdirectory(quill)

add_library(external INTERFACE)
target_link_libraries(external INTERFACE ${Boost_LIBRARIES} quill::quill)
target_include_directories(external INTERFACE ${Boost_INCLUDE_DIRS})