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

- The allocation of stacks is delayed until the scheduler (reactor) is ready to execute the fiber.
    - Can easily burn through a lot of memory especially if stacks are reused.
    - This delayed stack allocation enables reuse optimisation: keep a free-list of stacks, and reuse it for new fibers
      instead of `malloc`-ing new memory
- Not all fibers will require the same stack size. Can take inspiration from slab allocator design, keep a free-list for
  3-different stack sizes and allocate from that instead.

## Pinning of hardware threads

- We are the scheduler in this case. Don't want fibers to jump across cores, since we are scheduling them ourselves.
  Also incur cache penalties
- Clock drift: If we use `rdtsc` for timing, measured in terms of clock cycles, different cores will have clock drift
  and comparing cycle count is not fair. Using system calls is out of the question, too costly in terms of context
  switch.
- OS Specific
    - Windows: `SetThreadAffinityMask`
    - Linux: `pthread_setaffinity_np`

## Overflow queue

`kqueue` and `io_uring` themselves have a batch size. But if that is full, we
move the stuff into the overflow queue.
