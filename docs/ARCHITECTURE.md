# Design choices

Callback vs Threaded workers?
What's our problem?
We will have a few file descriptors which are subscribed to some exchange for streaming orderbook data.
We need to be able to read from the file descriptors and process them efficiently.
Do we need state persistence across function calls? The function that processes the websocket messages can essentially
be a state machine that is called every time a message is received. This function can be called from the event loop.

The coroutine runs until it yields cause it waits for an event

## Cooperative multi tasking

- The whole idea behind fibers is that instead of having time quantums, we trust that the tasks will yield within a
  reasonable amount of time. In exchange, we get full control over scheduling.
- The drawback of this approach is that any task ran by the event loop MUST NOT BLOCK. Disastrous things will happen if
  any task blocks.
    - Furthermore, once a task has started, we cannot pre-empt tasks as that would essentially become
      threading, and a whole suite of problems.

https://agraphicsguynotes.com/posts/fiber_in_cpp_understanding_the_basics/

## Memory Management for Fibers

- No runtime memory allocation.
  Following [Tiger Style](https://github.com/tigerbeetle/tigerbeetle/blob/main/docs/TIGER_STYLE.md) no memory can be
  freed and reallocated after initialisation. Avoid unpredictable behaviour and a system that is easier to reason about.
  Also affects performance significantly.
- The allocation of stacks is delayed until the scheduler (reactor) is ready to execute the fiber.
    - Can easily burn through a lot of memory especially if stacks are reused.
    - This delayed stack allocation enables reuse optimisation: keep a free-list of stacks, and reuse it for new fibers
      instead of `malloc`-ing new memory
- Not all fibers will require the same stack size. Can take inspiration from slab allocator design, keep a free-list for
  3-different stack sizes and allocate from that instead.

## M:1 Model & Pinning of hardware threads

- We are the scheduler in this case. Don't want fibers to jump across cores, since we are scheduling them ourselves.
  Also incur cache penalties
- Clock drift: If we use `rdtsc` for timing, measured in terms of clock cycles, different cores will have clock drift
  and comparing cycle count is not fair. Using system calls is out of the question, too costly in terms of context
  switch.
- OS Specific
    - Windows: `SetThreadAffinityMask`
    - Linux: `pthread_setaffinity_np`
- The event loop will be single threaded for now and pinned to a core. There are plans for work stealing, however.

## Overflow queue

`kqueue` and `io_uring` themselves have a batch size. But if that is full, we
move the stuff into the overflow queue.

## Timers

- The various IO readiness facilities all have different ways to schedule timers. `flux` aims to generalise this. There
  are two options:
    1. [Implement timers as a special lightweight fiber](https://photonlibos.github.io/docs/api/thread#timer) to
       periodically fire notifications. Good if we want to call functions on timer fires. Function signature can look
       like `Timer(uint64_t timeout, std::function<void*(void*)> on_timer, bool is_repeating = true, uint64_t stack_size = 1024)`
    2. Use the kernel facilities to register timers.

## Singleton Reactor

- As mentioned earlier, `flux` is a M:1 model. This calls for
  a [singleton pattern](https://photonlibos.github.io/docs/api/env), to ensure that the user doesn't accidentally
  create > 1 reactor.
- `void flux::init(flux::EventEngine event_engine = flux::EventEngine::AUTO);`