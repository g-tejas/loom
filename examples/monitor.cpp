#include <iostream>

#include "loom/all.hpp"

class Monitor : public loom::Fiber {
public:
    explicit Monitor(size_t stack_size) : loom::Fiber(stack_size) {}

    void run() override {
        while (true) {
            this->wait();
            std::cout << "File modified" << std::endl;
        }
    }
};

$LOOM_MAIN(int argc, char *argv[]) {
    LOOM_ASSERT(argc == 2, "Usage: monitor <file>");
    Monitor monitor(4096);
    monitor.start();

    loom::Kqueue kq;
    kq.subscribe_file(argv[1], &monitor);

    while (kq.active()) {
        kq.epoch();
    }

    return 0;
}