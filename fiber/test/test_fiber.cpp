#include <catch2/catch_test_macros.hpp>
#include <loom/fiber/fiber.h>

class TestFiber : public loom::Fiber {
public:
    explicit TestFiber(size_t stack_size) : loom::Fiber(stack_size), run_count(0) {}

    void run() override {
        ++run_count;
        if (run_count == 1) {
            wait();
        }
    }
    int run_count;
};

TEST_CASE("Fiber", "[fiber]") {
    TestFiber fiber(1024 * 64);

    SECTION("Fiber starts and runs") {
        REQUIRE(fiber.run_count == 0);
        fiber.start();
        REQUIRE(fiber.run_count == 1);
    }

    SECTION("Fiber waits and resumes") {
        fiber.start();
        REQUIRE(fiber.run_count == 1);

        loom::Event event{};
        bool resumed = fiber.resume(&event);
        REQUIRE(resumed == true);
        REQUIRE(fiber.run_count == 2);
    }
}