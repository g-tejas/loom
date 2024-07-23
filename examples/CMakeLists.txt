add_compile_definitions(USE_KQUEUE)

add_executable(fibers fibers.cpp)
target_link_libraries(fibers PRIVATE loom)

add_executable(timer timer.cpp)
target_link_libraries(timer PRIVATE loom)
