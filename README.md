# loom

Loom is a high performance, cross platform fiber event loop library for C++20.

This abstraction serves the main purpose: To not think about IO readiness events, but just IO completion.
What do I mean by this? The `io_uring` and `epoll` interface are quite different. `io_uring` takes the SQE,
together with a callback, and the kernel itself will perform the `read` for e.g. `epoll` on the other hand,
only notifies you of the readiness, and you would need to perform the `read` yourself.

This should be abstracted away. The user using this framework should not have to think about IO readiness events,
and the underlying implementation of the asynchronous interface.

## Usage

Loom requires that you hook your main method (e.g the same watch Catch2 does) in order to intercept system calls before
`glibc` functions are invoked. This has various purposes, mainly for proper scheduling behaviour and telemetry. For
example, we don't want a `sleep` in the fiber thread to block the entire thread.

> [!NOTE]
> Note that all macros in `loom` start with `$`.

```cpp
#include <loom/loom.h>

$LOOM_MAIN(int argc, char* argv[]) {
	int ret = loom::init(loom::EVENT_ENGINE_KQUEUE);
	LOOM_ASSERT(ret == 0, "Failed to initialize loom");
	defer(loom::fini());
}
```

## Features

- Cross platform: `kqueue`, `epoll`
- `glibc` system call hooks
- Fast context switches (powered by [Boost.Context](https://github.com/boostorg/context))
- (Eventually) No runtime allocations- Intrusive pointers
- Supports unix domain sockets
- Support for CPU affinity and pinning threads to cores

## Resources

- [Tiger Style](https://github.com/tigerbeetle/tigerbeetle/blob/main/docs/TIGER_STYLE.md)
- [⭐Fibers](https://graphitemaster.github.io/fibers/)
- [Writing high-performance servers](https://web.archive.org/web/20060306033511/https://pl.atyp.us/content/tech/servers.html)
- [kqueue documentation](https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/kqueue.2.html)
- [epoll documentation](https://man7.org/linux/man-pages/man7/epoll.7.html)

## Roadmap
- [ ] Inject tracy profiling before functions during build step using AST transformations
- [ ] Use `eventfd` for shared mem OB?
