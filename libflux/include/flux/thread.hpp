#pragma once

#include <boost/context/continuation.hpp>
#include <boost/context/detail/fcontext.hpp>
#include <boost/context/fiber.hpp>
#include <boost/context/protected_fixedsize_stack.hpp>
#include <boost/context/stack_context.hpp>
#include <tracy/Tracy.hpp>

#include <cstddef>
#include <iostream>
#include <vector>

#define FIBER_STATUS_COMPLETE 0
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
    SocketHangup = 4,
    Timer = 5,
  };
  Type type;
  int fd;
};

struct fiber_attr_t {
  size_t stack_size;
};

struct fiber_t {
  //! Represents the fiber's state, hardware (registers) state, etc
  fcontext_t m_fiber_ctx;
  //! Used for ctx switching
  ReturnContext m_ret_ctx;
  //! The stack size and pointer of this fiber
  stack_context m_stack_ctx;
  //! The file descriptors this fiber is subscribed to
  std::vector<int> m_fds;
};

struct fiber_args_t {
  fiber_t *fiber;
  void *(*start_routine)(void *);
  void *udata;
};

void fiber_enter(ReturnContext ctx);

/**
 * Inits the given fiber.
 *
 * @param fiber The reference to the fiber_t object to be initialised
 * @param attr The pointer to fiber_attr_t to specify attributes for the fiber. For e.g,
 * stack size, scheduling policy, etc
 * @param start_routine The function to be ran in the fiber
 * @param args The pointer to the argument to the start_routine
 * @return The status of the operation
 */
int fiber_init(fiber_t &fiber, const fiber_attr_t *attr, void *(*start_routine)(void *),
               void *args) {
  try {
    StackAllocator stk(attr->stack_size);
    fiber.m_stack_ctx = stk.allocate();

    // Allocate space for the fiber's control structure in the first few bytes of the
    // stack
    // reserve space for control structure at the top of the downward growing stack
    // void* sp Vx

    continuation return 0;
  } catch (const std::bad_alloc &e) {
    std::cout << "Failed to init\n";
    return -1;
  }
}

/**
 * The entry point to the fiber's context, must have this specific fn signature. Passed
 * to Boost's make_fcontext
 * @param ctx The context of the caller
 */
void fiber_enter(ReturnContext ctx) {
  auto *args = reinterpret_cast<fiber_args_t *>(ctx.data);

  auto *fiber = args->fiber;
  fiber->m_ret_ctx = ctx;

  auto ret = (int64_t)args->start_routine(args->udata);

  while (true) {
    // Transfer control back to the caller and pass zero to indicate that we are done
    fiber->m_ret_ctx = jump_fcontext(fiber->m_ret_ctx.fctx, (void *)ret);
  }
}

/**
 *
 * @param fiber
 */
void fiber_start(fiber_t &fiber) {
  // We enter the coroutine from a static routine because method signature of a
  // member function might be iffy, and we need to pass the `this` pointer.
  fiber.m_fiber_ctx =
      make_fcontext(fiber.m_stack_ctx.sp, fiber.m_stack_ctx.size, fiber_enter);

  // Transfers control to said fiber
  jump_fcontext(fiber.m_fiber_ctx, (void *)&fiber);
}

/**
 * De-initialises the given fiber and leaves it in a valid but unknown state.
 *
 * @param fiber The handle to the fiber to be de-initialised
 * @return The status of this operation
 */
inline void fiber_deinit(fiber_t &fiber) noexcept {
  StackAllocator stk(fiber.m_stack_ctx.size);
  stk.deallocate(fiber.m_stack_ctx);
}

/**
 * Resumes a fiber with the given event notification
 *
 * @param fiber The handle of the fiber to be resumed
 * @param event The event notification object
 * @return
 */
[[nodiscard]] auto fiber_resume(fiber_t &fiber, Event *event) -> bool {
  fiber.m_ret_ctx = jump_fcontext(fiber.m_ret_ctx.fctx, (void *)event);
  return fiber.m_ret_ctx.data != FIBER_STATUS_COMPLETE;
}

/**
 * Invoked from the fiber's context. It is resumed when an event notification occurs.
 *
 * @param fiber The handle of the fiber awaiting on a notification
 * @return A pointer to the event the fiber is awaiting
 */
auto fiber_wait(fiber_t &fiber) -> Event * {
  fiber.m_ret_ctx = jump_fcontext(fiber.m_ret_ctx.fctx, (void *)&fiber);
  return reinterpret_cast<Event *>(fiber.m_ret_ctx.data);
}

/// A stack-ful coroutine class. This is a base class and user needs to implement it
/// and implement the virtual `run()` method.
// TODO: Implement CRTP
// TODO: Implement boost intrusive pointer
// TODO: Implement pthread style creation instead of having to define an object
class Thread {
public:
  explicit Thread(size_t _stack_size);

  ~Thread();

  //! Called from within thread's context.
  //! Passes control back to the caller (e.g Reactor), and this thread will be resumed
  //! when the events are ready.
  auto wait() -> Event *;

  //! Resumes the thread with the given event. Returns true if resumable
  [[nodiscard]] auto resume(Event *event) -> bool;

  //! Where you place your business logic.
  virtual void run() = 0;

  //! Starts executing the `run()` method.
  void start();

  //! Subscribe to a particular file descriptor.
  auto subscribe(int fd) -> bool;

  //! Unsubscribe to a particular file descriptor.
  auto unsubscribe(int fd) -> bool;

  //! List of fds that this thread is interested in.
  std::vector<int> m_fds;

private:
  //! Entry point from the coroutine context, has the function type void* (*)(void*)
  static void enter(ReturnContext ctx);

  /// Represents the thread's state. Contains hardware context, stack pointer, instruction
  /// pointers, etc.
  fcontext_t m_thread_context{};

  /// Used for context switching. Flip flops between the reactor and the thread
  ReturnContext m_return_context{};

  /// Used for managing the stack of the thread
  stack_context m_stack;
};

} /* namespace flux */
