# Reactor Backends

## `kqueue` (Darwin)

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

## `io_uring` (Linux)

## `epoll` (Older Linux distributions)

