add_executable(test_fiber test_fiber.cpp)
target_link_libraries(test_fiber PRIVATE Catch2::Catch2WithMain loom)