# loom

This abstraction serves the main purpose: To not think about IO readiness events, but just IO completion.
What do I mean by this? The `io_uring` and `epoll` interface are quite different. `io_uring` takes the SQE,
together with a callback, and the kernel itself will perform the `read` for e.g. `epoll` on the other hand,
only notifies you of the readiness, and you would need to perform the `read` yourself.

This should be abstracted away. The user using this framework should not have to think about IO readiness events,
and the underlying implementation of the asynchronous interface.

## Features

- No runtime allocations
- Intrusive pointers
- Fast context switches (powered by Boost.Context)
- Multiple backends: `kqueue`, `epoll`, `io_uring` (all of these seem to be edge triggered)
- Supports unix domain sockets
- Support for pinning event loop to hardware threads.

## Todo

- Rename to `loom` ? More appropriate than `flux` since this is a fiber library.
  Need to decide whether we want a callback interface, or what.
  Normally, the callbacks are function pointers casted to integers, and stored in the user_data field of the SQE.
  Both kqueue and epoll have this. Once the IO is complete, the callback is invoked.

https://webflow.com/made-in-webflow/website/Apple-Style-Grid Can make this as the front page LOL

## Backends

1. Linux: `io_uring`, `epoll` (fallback incase kernel does not support `io_uring`)
2. MacOS: `kqueue`
   TODO: No support for Windows (`IOCP`) yet.

## Resources

- [Tiger Style](https://github.com/tigerbeetle/tigerbeetle/blob/main/docs/TIGER_STYLE.md)
- [⭐Fibers](https://graphitemaster.github.io/fibers/)
- [Old article about how to write high performance servers](https://web.archive.org/web/20060306033511/https://pl.atyp.us/content/tech/servers.html)
  Talks about the four horsemen of poor performance. Read if necessary
    - Context switches
    - Data copies
    - Lock contention
    - Memoy allocation

*`kqueue`*: https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/kqueue.2.html

- *`epoll`*: Can add the edge triggered and level triggered stuff also. Since we are using `epoll` and `io_uring`
  and `kqueue`,
  we just make the edge triggered the default.

## Ideas

- [ ] Inject tracy profiling before functions during build step using AST transformations
- [ ] Use `eventfd` for shared mem OB?
