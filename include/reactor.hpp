#pragma once

using Thread = int;

namespace flux {
//! Base class for all Reactor backends [`epoll`, `kqueue`, `io_uring`]
class Reactor {
public:
  Reactor() = default;
  virtual ~Reactor();

  //! Subscribe this thread to this file descriptor
  void subscribe(int fd, Thread thread);

  //! Remove all subscribers to a particular file descriptor
  void unsubscribe(int fd);

  //! Remove all subscribers to this thread
  void unsubscribe(Thread *thread);

  //! Returns true if any subscriptions are active
  auto active() -> bool const;

private:
};
} /* namespace flux */
