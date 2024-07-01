#include "fifo.hpp"
#include "utils.hpp"
#include <boost/context/detail/fcontext.hpp>
#include <boost/context/protected_fixedsize_stack.hpp>
#include <boost/context/stack_context.hpp>
#include <cassert>
#include <cstdio>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/event.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

// we check if macos
#ifdef __APPLE__
#define SOCK_NONBLOCK O_NONBLOCK
#endif

#define THREAD_STATUS_COMPLETE 0
#define THREAD_STATUS_ERROR 1

#define DEFAULT_STACK_SIZE 4096

namespace flux {
using namespace boost::context::detail;
using namespace boost::context;
using StackAllocator = protected_fixedsize_stack;
using ThreadContext = fcontext_t;
using ReturnContext = transfer_t;

struct Event {
  enum Type : uint8_t {
    NA = 0,
    SocketRead = 1,
    SocketWriteable = 2, // this is not currently implemented or handled
    SocketError = 3,
    SocketHangup = 4
  };
  Type type;
  int fd;
};

/// A stack-ful coroutine class. This is a base class and user needs to implement it
/// and implement the virtual `run()` method.
// TODO: Implement CRTP
// TODO: Implement boost intrusive pointer
// TODO: Implement pthread style creation instead of having to define an object
class Thread {
public:
  explicit Thread(size_t _stack_size) : m_stack_size(_stack_size) {
    protected_fixedsize_stack stack_allocator(m_stack_size);
    m_stack = stack_allocator.allocate();
  }
  ~Thread() {
    protected_fixedsize_stack stack_allocator(m_stack.size);
    stack_allocator.deallocate(m_stack);
  }

  /// Called from within thread's context
  /// Passes control back to the caller (e.g Reactor), and this thread will be resumed
  /// when the events are ready.
  auto wait() -> Event * {
    m_return_context = jump_fcontext(m_return_context.fctx, this);
    return reinterpret_cast<Event *>(m_return_context.data);
  }

  /// Resumes the thread with the given event. Returns true if resumable
  auto resume(Event *event) -> bool {
    m_return_context = jump_fcontext(m_thread_context, event);
    return m_return_context.data != THREAD_STATUS_COMPLETE;
  }

  /// Where you place your business logic
  virtual void run() = 0;

  /// Allocates `stack_size` for thread stack and starts executing the `run()` method
  void start(size_t stack_size) {
    // We enter the coroutine from a static routine because method signature of a
    // member function might be iffy, and we need to pass the `this` pointer.
    m_thread_context = make_fcontext(m_stack.sp, m_stack.size, Thread::enter);

    // Transfers control to this thread. The reason we pass `this` is that we want to
    // set the m_return_context to the reactor's context.
    jump_fcontext(m_thread_context, (void *)this);
  }

private:
  /// Entry point from the coroutine context, has the function type void* (*)(void*)
  static void enter(ReturnContext ctx) {
    auto *thread = reinterpret_cast<Thread *>(ctx.data);

    thread->m_return_context = ctx;

    thread->run();

    while (true) {
      // Transfer control back to the caller and pass zero to indicate that we are done
      thread->m_return_context = jump_fcontext(thread->m_return_context.fctx, 0);
    }
  }

private:
  /// Represents the thread's state. Contains hardware context, stack pointer, instruction
  /// pointers, etc.
  fcontext_t m_thread_context{};

  /// Used for context switching. Flip flops between the reactor and the thread
  ReturnContext m_return_context{};

  /// Used for managing the stack of the thread
  stack_context m_stack;

  /// Size of the requested stack
  size_t m_stack_size;
};
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