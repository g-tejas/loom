#pragma once

#include "flux/reactor.hpp"
#include <array>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>

namespace flux {
class KqueueReactor : public Reactor {
public:
  KqueueReactor();

  //! Closes the kqueue file descriptor
  ~KqueueReactor();

  //! This is one epoch of the event loop. Checks all the fds monitored by the event loop
  //! and dispatches any events accordingly.
  bool work();

  //! Runs the event loop for given time or until there are no subscriptions left,
  //! whichever comes first. Look at TigerBeetle's implementation
  auto run_for_ns(uint64_t ns) const -> bool;

  void set_timer(int id, int timer_period, Thread *thread);

private:
  //! Manages the socket operations. Basically a switch statement
  virtual void handle_sock_op(int fd, Operation op) override;

  //! How long to block waiting for events. `nullptr` means block indefinitely
  timespec *m_timeout;

  //! The file descriptor for the kqueue
  int m_kqueue_fd;

  std::array<struct kevent, 16> m_events{};

  //! Change list for the kqueue
  int m_nchanges;
  std::array<struct kevent, 16> m_changes{};
};
} /* namespace flux */
