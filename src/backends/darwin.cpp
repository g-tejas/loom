#include "backends/darwin.hpp"
#include "utils.hpp"

using namespace flux;

KqueueReactor::KqueueReactor() : m_kqueue_fd{kqueue()}, m_timeout{nullptr} {
  FLUX_ASSERT(m_kqueue_fd != -1, "Failed to create kqueue");
}

KqueueReactor::~KqueueReactor() { close(m_kqueue_fd); }

bool KqueueReactor::work() {
  // TODO: 16 can be adjusted based on busy-ness of the event loop. Can even be dynamic
  std::array<struct kevent, 16> events{};

  // changelist: List of events to register with the kernel
  // nchanges: Number of events in the changelist
  // `m_timeout` must be non-empty if we want to also monitor SPSC queues for example
  // `m_timeout` == 0 degrades the performance to a regular poll event loop
  int n = kevent(m_kqueue_fd, m_changes.data(), m_nchanges, events.data(), events.size(),
                 m_timeout);

  // `kevent` can be interrupted by signals, so we check `errno` global variable.
  if (n < 0 || errno == EINTR)
    return false;

  for (int i = 0; i < events.max_size(); ++i) {
    if (events[i].filter & EV_ERROR) {
      printf("Error event\n");
      //      this->notify(Event{.type = Event::Type::SocketError,
      //                         .fd = static_cast<int>(events[i].ident)});
    }
    if (events[i].flags & EVFILT_READ) {
      printf("Read event\n");
      this->notify(Event{.type = Event::Type::SocketRead,
                         .fd = static_cast<int>(events[i].ident)});
    } else if (events[i].flags & EVFILT_WRITE) {
      printf("Write event\n");
      this->notify(Event{.type = Event::Type::SocketWriteable,
                         .fd = static_cast<int>(events[i].ident)});
    } else if (events[i].flags & EVFILT_TIMER) {
      printf("Timer event\n");
      this->notify(Event{.type = Event::Type::Timer, .fd = -1});
    }
  }
  return true;
}

auto KqueueReactor::run_for_ns(uint64_t ns) const -> bool { return true; }

void KqueueReactor::set_timer(int timer_period) {
  static int timer_id = 100;
  struct kevent ev;

  EV_SET(&ev, timer_id++, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, timer_period, nullptr);
  m_subcount++;

  int n = kevent(m_kqueue_fd, &ev, 1, nullptr, 0, nullptr);
  FLUX_ASSERT(n >= 0, "Failed to set timer");
}

void KqueueReactor::handle_sock_op(int fd, Operation op) {
#ifndef NDEBUG
  FLUX_ASSERT(m_kqueue_fd >= 0, "Kqueue file descriptor is invalid");
#endif

  if (m_nchanges == m_changes.size()) [[unlikely]] {
    // `changelist` is full, need to flush the changes to the kernel.
    int n = kevent(m_kqueue_fd, m_changes.data(), m_nchanges, nullptr, 0, nullptr);
    FLUX_ASSERT(n >= 0, "Failed to flush changes to the kernel");
    m_nchanges = 0;
  }
  // EV_ADD attaches descriptor to kq
  // EV_CLEAR prevents unnecessary signalling
  switch (op) {
  case Operation::Added: {
    // Add RW flags to Operation as well
    EV_SET(&m_changes[m_nchanges++], fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, nullptr);
    EV_SET(&m_changes[m_nchanges++], fd, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, nullptr);
    break;
  }
  case Operation::Removed:
    EV_SET(&m_changes[m_nchanges++], fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
    EV_SET(&m_changes[m_nchanges++], fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
    break;
  case Operation::Modified: [[fallthrough]];
  case Operation::NIL: break;
  }
}
