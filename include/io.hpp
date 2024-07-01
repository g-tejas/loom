#pragma once

//! Base class for all Reactor backends (e.g `epoll`, `kqueue`, `io_uring`)
class Reactor {
public:
  Reactor() = default;
  virtual ~Reactor();
};