#pragma once

#include "loom/loom.hpp"
#include "loom/utils.hpp"

namespace loom {

Loom::Loom() {
    m_kqueue_fd = kqueue();
    FLUX_ASSERT(m_kqueue_fd != -1, "Failed to create kqueue");
}

Loom::~Loom() {
    if (m_kqueue_fd >= 0) {
        close(m_kqueue_fd);
    }
}

void Loom::handle_sock_op(int fd, loom::Operation op) {
#ifndef NDEBUG
    FLUX_ASSERT(m_kqueue_fd >= 0, "Kqueue file descriptor is invalid");
#endif

    if (m_changes.size() == 16) [[unlikely]] {
        // `changelist` is full, need to flush the changes to the kernel.
        int n =
            kevent(m_kqueue_fd, m_changes.data(), m_changes.size(), nullptr, 0, nullptr);
        FLUX_ASSERT(n >= 0, "Failed to flush changes to the kernel");
        m_changes.clear();
    }
    // EV_ADD attaches descriptor to kq
    // EV_CLEAR prevents unnecessary signalling
    struct kevent ev1 {};
    struct kevent ev2 {};
    switch (op) {
    case Operation::Added: {
        // Add RW flags to Operation as well
        // Add a new kevent to the vector
        EV_SET(&ev1, fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0, nullptr);
        EV_SET(&ev2, fd, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0, nullptr);
        m_changes.push_back(ev1);
        m_changes.push_back(ev2);
        break;
    }
    case Operation::Removed:
        EV_SET(&ev1, fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
        EV_SET(&ev2, fd, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
        break;
    case Operation::Modified: [[fallthrough]];
    case Operation::NIL: break;
    }
}

//! This is one epoch of the event loop. Checks all the fds monitored by the event loop
//! and dispatches any events accordingly.
bool Loom::epoch() {
    // TODO: 16 can be adjusted based on busy-ness of the event loop. Can even be dynamic
    std::array<struct kevent, 16> events{};

    // changelist: List of events to register with the kernel
    // nchanges: Number of events in the changelist
    // `m_timeout` must be non-empty if we want to also monitor SPSC queues for example
    // `m_timeout` == 0 degrades the performance to a regular poll event loop
    int n = kevent(m_kqueue_fd, m_changes.data(), m_changes.size(), events.data(),
                   events.size(), m_timeout);

    // `kevent` can be interrupted by signals, so we check `errno` global variable.
    if (n < 0 || errno == EINTR)
        return false;

    for (int i = 0; i < n; ++i) {
        if (events[i].flags & EV_ERROR) {
            printf("Error event\n");
        }
        if (events[i].filter == EVFILT_READ) {
            printf("Read event\n");
            this->notify(Event{.type = Event::Type::SocketRead,
                               .fd = static_cast<int>(events[i].ident)});
        } else if (events[i].filter == EVFILT_WRITE) {
            printf("Write event\n");
            this->notify(Event{.type = Event::Type::SocketWriteable,
                               .fd = static_cast<int>(events[i].ident)});
        } else if (events[i].filter == EVFILT_TIMER) {
            printf("Timer event\n");
            this->notify(Event{.type = Event::Type::Timer,
                               .fd = static_cast<int>(events[i].ident)});
        }
    }
    return true;
}

void Loom::set_timer(int id, int timer_period) {
    FLUX_ASSERT(m_kqueue_fd >= 0, "Kqueue file descriptor is invalid");
    struct kevent ev {};
    EV_SET(&ev, id, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, timer_period, nullptr);
    int n = kevent(m_kqueue_fd, &ev, 1, nullptr, 0, nullptr);
    FLUX_ASSERT(n >= 0, "Failed to set timer");
}
} /* namespace loom */
