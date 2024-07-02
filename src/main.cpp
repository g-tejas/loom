#include "fifo.hpp"
#include "utils.hpp"
#include <cstdio>
#include <fcntl.h>

#include "thread.hpp"

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

// struct Completion {
//   Completion *next;
//   //  void (*callback)(IO *, Completion *);
// };
//
// class IO {
// private:
//   Fifo<Completion> timeouts;
//   Fifo<Completion> completed;
//   Fifo<Completion> io_pending;
//
// public:
//   IO(size_t entries, int flags) {}
//
//   ~IO() = default;
//
// public:
//   // Pass all queued submissions to the kernel and peek for completions
//   void tick() {}
//
//   // Pass all queued submissions to the kernel and run for `nanoseconds`
//   void run_for_ns(uint64_t nanoseconds) {}
//
//   void flush(bool wait_for_completions) {}
//
// public:
//   void accept() {}
//
//   void close() {}
//
//   void connect() {}
//
//   void read() {}
//
//   void recv() {}
//
//   void send() {}
//
//   void write() {}
//   void timeout() {}
//
// private:
//   void submit() {}
// };

struct Worker : flux::Thread {
  explicit Worker(size_t _stack_size) : Thread(_stack_size) {}
  void run() override {
    int count = 0;
    int thing = 0;
    while (count < 100) {
      count++;
      thing = (thing * 7 + count) / 8;
    }
  }
};

int main() {
  // Create a new context and stack to execute foo. Pass the stack pointer to the end,
  // stack grows downwards for most architecture, from highest mem address -> lowest
  Worker worker(DEFAULT_STACK_SIZE);
  worker.run();

  flux::Event fake{flux::Event::Type::NA, 0};

  while (worker.resume(&fake)) {
  }

  printf("Done\n");
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