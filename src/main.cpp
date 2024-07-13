#include "fifo.hpp"
#include "utils.hpp"
#include <cstdio>
#include <fcntl.h>
#include <iostream>

#include "backends/darwin.hpp"
#include "thread.hpp"

using namespace std;

// we check if macos
#ifdef __APPLE__
#define SOCK_NONBLOCK O_NONBLOCK
#endif

#define DEFAULT_STACK_SIZE 4096

namespace flux {

// I want the API to look like this somehow. Not a class that i have to inherit
// and then write the logic in the run code. Need to see how can modify this to run a
// function instead.
// Reference implementation: https://photonlibos.github.io/docs/api/thread#thread_create
void thread_create(void *(*fn)(void *), void *arg,
                   uint64_t stack_size = DEFAULT_STACK_SIZE);
} /* namespace flux */

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
    }
  }
};
int main() {
  // Create a new context and stack to execute foo. Pass the stack pointer to the end,
  // stack grows downwards for most architecture, from highest mem address -> lowest
  Worker worker(DEFAULT_STACK_SIZE); // need to heap allocate
  worker.start();
  flux::Event fake{flux::Event::Type::NA, 0};

  flux::KqueueReactor reactor;
  reactor.set_timer(1010, 1500, &worker);
  //  reactor.subscribe(1010, &worker);

  while (reactor.active()) {
    reactor.work();
  }

  return 0;

  //  CHECK_EX(false, "This is a test");
  //  exit(1);
  //  kq = kqueue();
  //  assert(kq != -1);
  //
  //  struct context obj = {};
  //  obj.rhandler = [](struct context *obj) {
  //    printf("Received socket READ event via kqueue\n");
  //    int csock = accept(obj->sk, NULL, 0);
  //    assert(csock != -1);
  //    close(csock);
  //  };
  //
  //  // creet and prepare a socket
  //  obj.sk = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  //  assert(obj.sk != -1);
  //  int val = 1;
  //  setsockopt(obj.sk, SOL_SOCKET, SO_REUSEADDR, &val, 4);
  //
  //  struct sockaddr_in addr = {};
  //  addr.sin_family = AF_INET;
  //  addr.sin_port = ntohs(64000);
}
