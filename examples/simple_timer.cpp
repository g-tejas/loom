#include <iostream>

#include "loom/all.hpp"

using namespace loom;

class Worker : public loom::Fiber {
public:
    explicit Worker(size_t stack_size) : loom::Fiber(stack_size) {}

    void run() override {
        while (true) {
            auto *event = wait();
            if (event->type == loom::Event::Type::Timer) {
                std::cout << "Timer event fired at " << time(nullptr) << "\n";
            } else {
                std::cout << "Unknown event type: " << static_cast<int>(event->type)
                          << "\n";
            }
        }
    }
};

$LOOM_MAIN(int argc, char *argv[]) {
    std::cout << "Hello\n";
    Worker worker(4096);
    worker.start();

    auto loom = loom::get();
    //    loom::Kqueue loom;
    loom.set_timer(10, 1, &worker);

    while (loom.active()) {
        loom.epoch();
    }
}
