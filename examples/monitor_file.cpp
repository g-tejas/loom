#include "loom/common/utils.h"
#include "loom/io/engine.h"
#include <iostream>
#include <loom/loom.h>
#include <sys/fcntl.h>

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

    int ret = loom::init();
    LOOM_ASSERT(ret == 0, "Failed to initialize loom");
    defer(loom::fini());

    int fd = open(argv[1], O_EVTONLY);
    LOOM_ASSERT(fd >= 0, "Failed to open file");

    loom::subscribe(fd, &monitor);

    // kq.subscribe_file(argv[1], &monitor);

    // while (kq.active()) {
    //     kq.epoch();
    // }

    return 0;
}
