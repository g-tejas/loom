#include "fifo.hpp"
#include "utils.hpp"
#include <assert.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/event.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

// we check if macos
#ifdef __APPLE__
#define SOCK_NONBLOCK O_NONBLOCK
#endif

class IO {
public:
  IO(size_t entries, int flags) {}

  ~IO() = default;

public:
  // Pass all queued submissions to the kernel and peek for completions
  void tick() {}

  // Pass all queued submissions to the kernel and run for `nanoseconds`
  void run_for_ns(uint64_t nanoseconds) {}

  void flush(bool wait_for_completions) {}

public:
  void accept() {}

  void close() {}

  void connect() {}

  void read() {}

  void recv() {}

  void send() {}

  void write() {}
  void timeout() {}

private:
  void submit() {}
};

struct Completion {
  Completion *next;

  void (*callback)(IO *, Completion *);
};

int kq;

// Sort of the mother socket, the socket from which we create all other sockets
// by calling accept on it
struct context {
  int sk;

  void (*rhandler)(struct context *obj);
};

int main() {
  kq = kqueue();
  assert(kq != -1);

  struct context obj = {};
  obj.rhandler = [](struct context *obj) {
    printf("Received socket READ event via kqueue\n");
    int csock = accept(obj->sk, NULL, 0);
    assert(csock != -1);
    close(csock);
  };

  // creet and prepare a socket
  obj.sk = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
  assert(obj.sk != -1);
  int val = 1;
  setsockopt(obj.sk, SOL_SOCKET, SO_REUSEADDR, &val, 4);

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(64000);
}