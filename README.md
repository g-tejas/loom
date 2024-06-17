# eventloop

This abstraction serves the main purpose: To not think about IO readiness events, but just IO completion.
What do I mean by this? The `io_uring` and `epoll` interface are quite different. `io_uring` takes the SQE, 
together with a callback, and the kernel itself will perform the `read` for e.g. `epoll` on the other hand,
only notifies you of the readiness, and you would need to perform the `read` yourself. 

This should be abstracted away. The user using this framework should not have to think about IO readiness events,
and the underlying implementation of the asynchronous interface.

## Priorities
- No runtime allocations
- 


## Todo
Need to decide whether we want a callback interface, or what.
Normally, the callbacks are function pointers casted to integers, and stored in the user_data field of the SQE. 
Both kqueue and epoll have this. Once the IO is complete, the callback is invoked.


## Backends
1. Linux: `io_uring`, `epoll` (fallback incase kernel does not support `io_uring`)
2. MacOS: `kqueue` 
TODO: No support for Windows (`IOCP`) yet. 

## Resources
*`kqueue`*
- https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/kqueue.2.html

*`epoll`*