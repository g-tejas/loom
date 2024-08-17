add_executable(test_fifo test_fifo.cpp)
target_link_libraries(test_fifo PRIVATE Catch2::Catch2WithMain loom)