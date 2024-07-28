#include <iostream>

#include "loom/all.hpp"
#include <dlfcn.h>

using namespace std;

struct Worker : loom::Fiber {
    explicit Worker(size_t stack_size) : loom::Fiber(stack_size) {}
    void run() override {
        int thing = 0;
        for (int i = 0; i < 100; ++i) {
            thing = (thing * 7 + i) / 8;
            printf("count: %d\n", i);
            this->wait();
        }
    }
};

$LOOM_MAIN(int argc, char *argv[]) {
    Worker worker(4096);
    worker.start();
    loom::Event fake{loom::Event::Type::NA, 0};

    while (worker.resume(&fake)) {
        cout << "Resuming worker" << endl;
    }
    return 0;
}
