#include <cstdio>
#include <fcntl.h>

#include "tracy/Tracy.hpp"
#include "tracy/TracyC.h"

#include "flux/backends/darwin.hpp"
#include "flux/thread.hpp"

using namespace std;

#ifdef __APPLE__
#define SOCK_NONBLOCK O_NONBLOCK
#endif

#define DEFAULT_STACK_SIZE 4096

struct Worker : flux::Thread {
  explicit Worker(size_t _stack_size) : Thread(_stack_size) {}
  void run() override {
    int count = 0;
    int thing = 0;
    while (count < 100) {
      count++;
      thing = (thing * 7 + count) / 8;
      printf("count: %d\n", count);
      this->wait();
      FrameMark;
    }
  }
};

void *print(void *args) {
  int i = 0;
  while (i++) {
    std::cout << i << ": hello world\n";
  }
}

int main() {
  //  // Create a new context and stack to execute foo. Pass the stack pointer to the end,
  //  // stack grows downwards for most architecture, from highest mem address -> lowest
  //  Worker worker(DEFAULT_STACK_SIZE); // need to heap allocate
  //  worker.start();
  //  flux::Event fake{flux::Event::Type::NA, 0};
  //
  //  flux::KqueueReactor reactor;
  //  reactor.set_timer(1010, 1500, &worker);
  //  //  reactor.subscribe(1010, &worker);
  //  tracy::SetThreadName("hello");
  //
  //  while (reactor.active()) {
  //    reactor.work();
  //    FrameMark;
  //  }

  flux::fiber_t f1;
  flux::fiber_init(f1, nullptr, print, nullptr);
}
