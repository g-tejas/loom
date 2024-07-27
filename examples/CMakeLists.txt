message(STATUS "${CMAKE_PROJECT_NAME}: Building examples")

add_executable(fibers fibers.cpp)
target_link_libraries(fibers PRIVATE loom)

add_executable(timer timer.cpp)
target_link_libraries(timer PRIVATE loom)

add_executable(monitor monitor.cpp)
target_link_libraries(monitor PRIVATE loom)
