message(STATUS "${CMAKE_PROJECT_NAME}: Building examples")

add_executable(simple_fiber simple_fiber.cpp)
target_link_libraries(simple_fiber PRIVATE loom)

#add_executable(simple_timer simple_timer.cpp)
#target_link_libraries(simple_timer PRIVATE loom)

add_executable(monitor_file monitor_file.cpp)
target_link_libraries(monitor_file PRIVATE loom)
