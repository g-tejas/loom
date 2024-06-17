add_subdirectory(Catch2)

add_library(external INTERFACE)
target_link_libraries(external INTERFACE Catch2::Catch2WithMain)
