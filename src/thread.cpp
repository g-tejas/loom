#include "thread.hpp"

using namespace flux;

Thread::Thread(size_t _stack_size) {
  StackAllocator stack_allocator(_stack_size);
  m_stack = stack_allocator.allocate();
}

Thread::~Thread() {
  StackAllocator stack_allocator(m_stack.size);
  stack_allocator.deallocate(m_stack);
}

auto Thread::wait() -> Event * {
  m_return_context = jump_fcontext(m_return_context.fctx, this);
  return reinterpret_cast<Event *>(m_return_context.data);
}

auto Thread::resume(Event *event) -> bool {
  printf("in resume\n");
  m_return_context = jump_fcontext(m_return_context.fctx, (void *)event);
  return m_return_context.data != THREAD_STATUS_COMPLETE;
}

void Thread::start() {
  // We enter the coroutine from a static routine because method signature of a
  // member function might be iffy, and we need to pass the `this` pointer.
  m_thread_context = make_fcontext(m_stack.sp, m_stack.size, Thread::enter);

  // Transfers control to this thread. The reason we pass `this` is that we want to
  // set the m_return_context to the reactor's context.
  m_return_context = jump_fcontext(m_thread_context, (void *)this);
}

void Thread::enter(ReturnContext ctx) {
  auto *thread = reinterpret_cast<Thread *>(ctx.data);

  thread->m_return_context = ctx;

  thread->run();

  while (true) {
    // Transfer control back to the caller and pass zero to indicate that we are done
    thread->m_return_context = jump_fcontext(thread->m_return_context.fctx, 0);
  }
}

auto Thread::subscribe(int fd) -> bool {
  auto it = std::find(m_fds.begin(), m_fds.end(), fd);
  if (it != m_fds.end()) {
    return false;
  }
  m_fds.push_back(fd);
  return true;
}

auto Thread::unsubscribe(int fd) -> bool {
  auto it = std::find(m_fds.begin(), m_fds.end(), fd);
  if (it != m_fds.end()) {
    m_fds.erase(it);
    return true;
  }
  return false;
}