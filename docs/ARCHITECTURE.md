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

## Design choices

Callback vs Threaded workers?
What's our problem?
We will have a few file descriptors which are subscribed to some exchange for streaming orderbook data.
We need to be able to read from the file descriptors and process them efficiently.
Do we need state persistence across function calls? The function that processes the websocket messages can essentially
be a state machine that is called every time a message is received. This function can be called from the event loop.

The coroutine runs until it yields cause it waits for an event

# Boost.Context

# Backends

## Kqueue

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

