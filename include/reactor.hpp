#pragma once

#include "thread.hpp"
#include <unordered_map>

using Thread = int;

namespace flux {
//! Abstract base class for all Reactor backends [`epoll`, `kqueue`, `io_uring`]
class Reactor {
public:
  Reactor() = default;
  virtual ~Reactor();

  //! Subscribe this thread to this file descriptor
  void subscribe(int fd, Thread *thread);

  //! Remove all subscribers to a particular file descriptor
  void remove_socket(int fd);

  //! Remove all subscribers to this thread
  void remove_thread(Thread *thread);

  //! Returns true if any subscriptions are active
  [[nodiscard]] auto active() const -> bool;

  //! Notify the subscribers of this event about the event. Will cause a context switch.
  void notify(Event event);

protected:
  enum class Operation : uint8_t {
    NIL = 0,
    Added = 1,
    Removed = 2,
    Modified = 3,
  };

  struct Subscription {
    int fd;
    Thread *thread;
  };

  std::unordered_map<int, std::vector<Thread *>> m_subs_by_fd;
  int m_subcount;

  //! Tells the kernel queue of choice to start / stop monitoring the file descriptor
  virtual void handle_sock_op(int fd, Operation op) = 0;

private:
};
} /* namespace flux */
