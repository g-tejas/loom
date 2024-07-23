#pragma once

#include <boost/context/continuation.hpp>
#include <boost/context/detail/fcontext.hpp>
#include <boost/context/fiber.hpp>
#include <boost/context/protected_fixedsize_stack.hpp>
#include <boost/context/stack_context.hpp>
#include <tracy/Tracy.hpp>

#include "loom/event.hpp"

#include <cstddef>
#include <iostream>
#include <vector>

#define FIBER_STATUS_COMPLETE 0
#define THREAD_STATUS_ERROR 1

namespace loom {
namespace ctx = boost::context;

using ThreadContext = ctx::detail::fcontext_t;
using ReturnContext = ctx::detail::transfer_t;
using StackAllocator = ctx::protected_fixedsize_stack;
using StackContext = ctx::stack_context;

/// A stack-ful coroutine class. This is a base class and user needs to implement it
/// and implement the virtual `run()` method.
class Fiber {
public:
    explicit Fiber(size_t _stack_size) {
        StackAllocator allocator(_stack_size);
        m_stack = allocator.allocate();
    }

    ~Fiber() {
        StackAllocator stack_allocator(m_stack.size);
        stack_allocator.deallocate(m_stack);
    }

    Fiber(const Fiber &) = delete;
    Fiber &operator=(const Fiber &) = delete;

    Fiber(Fiber &&) = delete;
    Fiber &operator=(Fiber &&) = delete;

    /**
     * The equivalent of `await`. Passes control back to the caller (e.g Loom) and will be
     * resumed when the event notification happens
     * @return
     */
    auto wait() -> Event * {
        m_return_context = ctx::detail::jump_fcontext(m_return_context.fctx, this);
        return reinterpret_cast<Event *>(m_return_context.data);
    }

    //! Resumes the fiber with the given event. Returns true if resumable
    [[nodiscard]] auto resume(Event *event) -> bool {
        m_return_context =
            ctx::detail::jump_fcontext(m_return_context.fctx, (void *)event);
        return m_return_context.data != FIBER_STATUS_COMPLETE;
    }

    //! Where you place your business logic.
    virtual void run() = 0;

    //! Starts executing the `run()` method.
    void start() {
        // We enter the coroutine from a static routine because method signature of a
        // member function might be iffy, and we need to pass the `this` pointer.
        m_thread_context = make_fcontext(m_stack.sp, m_stack.size, Fiber::enter);

        // Transfers control to this thread. The reason we pass `this` is that we want to
        // set the m_return_context to the reactor's context.
        m_return_context = ctx::detail::jump_fcontext(m_thread_context, (void *)this);
    }

    //! List of fds that this fiber is interested in.
    std::vector<int> m_fds;

private:
    //! Entry point from the coroutine context, has the function type void* (*)(void*)
    static void enter(ReturnContext ctx) {
        auto *thread = reinterpret_cast<Fiber *>(ctx.data);

        thread->m_return_context = ctx;
        thread->run();

        while (true) {
            // Transfer control back to the caller and pass zero to indicate that we are
            // done
            thread->m_return_context = ctx::detail::jump_fcontext(
                thread->m_return_context.fctx, FIBER_STATUS_COMPLETE);
        }
    }

    /// Represents the fiber's state. Contains hardware context, stack pointer,
    /// instruction pointers, etc.
    ctx::detail::fcontext_t m_thread_context{};

    /// Used for context switching. Flip flops between the reactor and the fiber
    ReturnContext m_return_context{};

    /// Used for managing the stack of the fiberf
    StackContext m_stack;
};

} /* namespace loom */
