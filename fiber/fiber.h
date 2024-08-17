#pragma once

#include <boost/context/detail/fcontext.hpp>
#include <boost/context/fiber.hpp>
#include <boost/context/protected_fixedsize_stack.hpp>
#include <boost/context/stack_context.hpp>
#include <tracy/Tracy.hpp>

#include <loom/io/event.h>

#include <cstddef>
#include <iostream>
#include <vector>

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
    explicit Fiber(size_t _stack_size);
    ~Fiber();
    Fiber(const Fiber &) = delete;
    Fiber &operator=(const Fiber &) = delete;
    Fiber(Fiber &&) = delete;
    Fiber &operator=(Fiber &&) = delete;

    /**
     * The equivalent of `await`. Passes control back to the caller (e.g Loom) and will be
     * resumed when the event notification happens
     * @return
     */
    auto wait() -> Event *;

    //! Resumes the fiber with the given event. Returns true if resumable
    [[nodiscard]] auto resume(Event *event) -> bool;

    //! Where you place your business logic.
    virtual void run() = 0;

    //! Starts executing the `run()` method.
    void start();

    //! List of fds that this fiber is interested in.
    std::vector<int> m_fds;

private:
    //! Entry point from the coroutine context, has the function type void* (*)(void*)
    static void enter(ReturnContext ctx);

    /// Represents the fiber's state. Contains hardware context, stack pointer,
    /// instruction pointers, etc.
    ctx::detail::fcontext_t m_thread_context{};

    /// Used for context switching. Flip flops between the reactor and the fiber
    ReturnContext m_return_context{};

    /// Used for managing the stack of the fiber
    StackContext m_stack;
};

} /* namespace loom */
