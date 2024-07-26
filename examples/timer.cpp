#include <iostream>

#include "loom/all.hpp"

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

int main() {
    Worker worker(4096);
    worker.start();

    loom::Kqueue loom;
    loom.set_timer(10, 1, &worker);

    while (loom.active()) {
        loom.epoch();
    }
}