#include "backends/darwin.hpp"
#include "utils.hpp"

using namespace flux;

KqueueReactor::KqueueReactor() : m_kqueue_fd{kqueue()}, m_timeout{nullptr} {
  FLUX_ASSERT(m_kqueue_fd != -1, "Failed to create kqueue");
}

KqueueReactor::~KqueueReactor() {
  // TODO: Need to close all the sockets as well
  close(m_kqueue_fd);
}

bool KqueueReactor::work() {
  // 16 can be adjusted based on busy-ness of the event loop. Can even be dynamic
  std::array<struct kevent, 16> events{};

  // changelist: List of events to register with the kernel
  // nchanges: Number of events in the changelist
  // `m_timeout` must be non-empty if we want to also monitor SPSC queues for example
  // `m_timeout` == 0 degrades the performance to a regular poll event loop
  int n = kevent(m_kqueue_fd, m_changes.data(), m_nchanges, events.data(), events.size(),
                 m_timeout);

  // `kevent` can be interrupted by signals., so we check `errno` global variable.
  if (n < 0 || errno == EINTR)
    return false;

  FLUX_ASSERT(n > 0, "kevent failed");

  for (int i = 0; i < 16; ++i) {
    if (events[i].filter & EV_ERROR) {
      this->notify(Event{.type = Event::Type::SocketError, .fd = 1});
    }
    switch (events[i].filter) {
    case EVFILT_READ:
      this->notify(Event{.type = Event::Type::SocketRead, .fd = 1});
      break;
    case EVFILT_WRITE:
      // Handle write event
      break;
    }
  }
}

bool KqueueReactor::run_for_ns(uint64_t ns) {}

void KqueueReactor::handle_sock_op(int fd, Operation op) {
#ifndef NDEBUG
  FLUX_ASSERT(m_kqueue_fd >= 0, "Kqueue file descriptor is invalid");
#endif

  if (m_nchanges == m_changes.size()) [[unlikely]] {
    // We need to flush the changes to the kernel. Return 0 timeout for instant return? We
    // don't want this fn to block
    int n = kevent(m_kqueue_fd, m_changes.data(), m_nchanges, nullptr, 0, nullptr);
    FLUX_ASSERT(n >= 0, "Failed to flush changes to the kernel");
    m_nchanges = 0;
  }
  // EV_ADD attaches descriptor to kq
  // EV_CLEAR prevents unnecessary signalling
  switch (op) {
  case Operation::Added: {
    // Can probably split operation up into more fine grained operations.
    EV_SET(&m_changes[m_nchanges++], fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, nullptr);
    EV_SET(&m_changes[m_nchanges++], fd, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, nullptr);
    break;
  }
  case Operation::Removed: break;
  case Operation::Modified: [[fallthrough]];
  case Operation::NIL: break;
  }
}
