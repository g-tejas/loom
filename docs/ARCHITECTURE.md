Any task that is ran by the event loop MUST NOT BLOCK. Disastrous things will
happen if any task blocks. Furthermore, once a task has started, we cannot pre-empt
tasks as that would essentially become threading, and a whole suite of problems.

In the case of CPU intensive tasks, a threaded based program would perform
much better.

We might have to use threaded workers for filesystem operations, if `io_uring` 
is not available. We can also use this to offload CPU intensive tasks. TODO: Perhaps,
for us, the orderbook building can be offloaded? Does this mean we need a separate `submit` queue?


## eventloop interface
```cpp
event_loop.write(fd, buffer, bytesToWrite, callback);
char buffer[1024];
event_loop.read(fd, &buffer, bytesToRead, callback);
```
`kqueue` and `io_uring` themselves have a batch size. But if that is full, we 
move the stuff into the overflow queue.

Overflow queue. 