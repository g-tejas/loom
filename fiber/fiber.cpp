#pragma once

#include <boost/context/detail/fcontext.hpp>
#include <boost/context/protected_fixedsize_stack.hpp>

#include <loom/fiber/fiber.h>
#include <loom/io/event.h>

namespace loom {

Fiber::Fiber(size_t _stack_size) {
    StackAllocator allocator(_stack_size);
    m_stack = allocator.allocate();
}

Fiber::~Fiber() {
    StackAllocator stack_allocator(m_stack.size);
    stack_allocator.deallocate(m_stack);
}

auto Fiber::wait() -> Event * {
    m_return_context = ctx::detail::jump_fcontext(m_return_context.fctx, this);
    return reinterpret_cast<Event *>(m_return_context.data);
}

auto Fiber::resume(Event *event) -> bool {
    m_return_context = ctx::detail::jump_fcontext(m_return_context.fctx, (void *)event);
    return m_return_context.data != 0;
}

void Fiber::start() {
    // We enter the coroutine from a static routine because method signature of a
    // member function might be iffy, and we need to pass the `this` pointer.
    m_thread_context = make_fcontext(m_stack.sp, m_stack.size, Fiber::enter);

    // Transfers control to this thread. The reason we pass `this` is that we want to
    // set the m_return_context to the reactor's context.
    m_return_context = ctx::detail::jump_fcontext(m_thread_context, (void *)this);
}

void Fiber::enter(ReturnContext ctx) {
    auto *thread = reinterpret_cast<Fiber *>(ctx.data);

    thread->m_return_context = ctx;
    thread->run();

    while (true) {
        // Transfer control back to the caller and pass zero to indicate that we are
        // done
        thread->m_return_context =
            ctx::detail::jump_fcontext(thread->m_return_context.fctx, 0);
    }
}

} /* namespace loom */