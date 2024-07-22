add_compile_definitions(USE_KQUEUE)

add_executable(fibers fibers.cpp)
target_link_libraries(fibers PRIVATE loom)

add_executable(monitor monitor.cpp)
target_link_libraries(monitor PRIVATE loom)
