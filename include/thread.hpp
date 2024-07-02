#pragma once

#include <boost/context/detail/fcontext.hpp>
#include <boost/context/protected_fixedsize_stack.hpp>
#include <boost/context/stack_context.hpp>

#include <cstddef>

#define THREAD_STATUS_COMPLETE 0
#define THREAD_STATUS_ERROR 1

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
  explicit Thread(size_t _stack_size);

  ~Thread();

  //! Called from within thread's context
  //! Passes control back to the caller (e.g Reactor), and this thread will be resumed
  //! when the events are ready.
  auto wait() -> Event *;

  //! Resumes the thread with the given event. Returns true if resumable
  auto resume(Event *event) -> bool;

  //! Where you place your business logic
  virtual void run() = 0;

  //! Allocates `stack_size` for thread stack and starts executing the `run()` method
  void start(size_t stack_size);

private:
  //! Entry point from the coroutine context, has the function type void* (*)(void*)
  static void enter(ReturnContext ctx);

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

} /* namespace flux */
