# Reactor Backends

[Super comprehensive blog](https://habr.com/en/articles/600123/) post on several event notification
backends: `kqueue`,`epoll`, `IOCP`

## `kqueue` (Darwin)

[Bible](https://people.freebsd.org/~jlemon/papers/kqueue.pdf)
`KV_CLEAR`: Prevents kq from signalling the same event over and over again. If a socket wasn't read fully,
then kq will signal us again, so instead of having to process the same signal multiple times, we make sure that we
fully consume the data.

The `poll` interface requires passing one big array containing all the fds in each event loop epoch. A `kqueue`
event loop on the other hand notifies the kernel of changes to the monitored fds by passing in a changelist. This
can be done in two ways,

1. Call `kevent` for each change to actively monitored fd list
2. Build up a list of descriptor changes and pass it to the kernel the next event loop epoch. Reduces number of
   sys-calls made

```cpp
struct kevent {
  uintptr_t ident; // Most of the time refers to a FD, but its meaning can change based on the filter. Essentially a value to identify the event
  filter; // Determines the kernel filter to process this event, e.g `EVFILT_TIMER`

};
```

https://stackoverflow.com/questions/37731435/what-exactly-is-kqueues-ev-receipt-for
EV_RECEIPT

## `io_uring` (Linux)

https://github.com/axboe/liburing/issues/536
https://github.com/axboe/liburing/issues/189
https://blog.cloudflare.com/missing-manuals-io_uring-worker-pool

## `epoll` (Older Linux distributions)

# Boost.Context

[Docs](https://live.boost.org/doc/libs/1_53_0/libs/context/doc/html/context/context.html#context.context.executing_a_context)

- UB if a context function throws an exception. Wrap with `try-catch`
- Calling `jump_fcontext` to the same context you are within results in undefined behaviour.
- The size of the stack must be larger than `fcontext_t`

There are POSIX apis for `makecontext`/`swapcontext`/`getcontext`. Perhaps we can wrap the Boost.Context fns around
these. So that once time permits, we
can [implement context switches with hand written asm](https://graphitemaster.github.io/fibers/). Boost.Context was
written to be general and cross platform, so we could probably strip out a lot of the stuff to optimise.

Use cases of Boost.Context in the wild

- [Facebook's Folly](https://github.com/facebook/folly/blob/main/folly/fibers/Fiber.h) Actually really good, look at it
  for fiber design. The entire fiber library is built around their `baton::post` and `baton::wait`
