add_executable(bench_fifo bench_fifo.cpp)
target_link_libraries(bench_fifo PRIVATE benchmark::benchmark loom)
