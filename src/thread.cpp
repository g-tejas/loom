#include "thread.hpp"

using namespace flux;

Thread::Thread(size_t _stack_size) : m_stack_size(_stack_size) {
  protected_fixedsize_stack stack_allocator(m_stack_size);
  m_stack = stack_allocator.allocate();
}

Thread::~Thread() {
  protected_fixedsize_stack stack_allocator(m_stack.size);
  stack_allocator.deallocate(m_stack);
}

auto Thread::wait() -> Event * {
  m_return_context = jump_fcontext(m_return_context.fctx, this);
  return reinterpret_cast<Event *>(m_return_context.data);
}

auto Thread::resume(Event *event) -> bool {
  m_return_context = jump_fcontext(m_thread_context, event);
  return m_return_context.data != THREAD_STATUS_COMPLETE;
}

void Thread::start(size_t stack_size) {
  // We enter the coroutine from a static routine because method signature of a
  // member function might be iffy, and we need to pass the `this` pointer.
  m_thread_context = make_fcontext(m_stack.sp, m_stack.size, Thread::enter);

  // Transfers control to this thread. The reason we pass `this` is that we want to
  // set the m_return_context to the reactor's context.
  jump_fcontext(m_thread_context, (void *)this);
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
