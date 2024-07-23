#pragma once

#include "loom/loom.hpp"
#include "loom/utils.hpp"

#include <format>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <sys/event.h>

std::string flagstring(uint32_t flags) {
    std::ostringstream ret;
    bool first = true;

    const std::pair<int, const char *> flag_names[] = {
        {NOTE_DELETE, "NOTE_DELETE"}, {NOTE_WRITE, "NOTE_WRITE"},
        {NOTE_EXTEND, "NOTE_EXTEND"}, {NOTE_ATTRIB, "NOTE_ATTRIB"},
        {NOTE_LINK, "NOTE_LINK"},     {NOTE_RENAME, "NOTE_RENAME"},
        {NOTE_REVOKE, "NOTE_REVOKE"}};

    for (const auto &[flag, name] : flag_names) {
        if (flags & flag) {
            if (!first) {
                ret << "|";
            }
            ret << name;
            first = false;
        }
    }

    return ret.str();
}
template <>
struct std::formatter<struct kevent> {
    constexpr auto parse(auto &ctx) { return ctx.begin(); }

    auto format(const struct kevent &kev, auto &ctx) const {
        return std::format_to(ctx.out(),
                              "kevent{{ ident: {}, filter: {}, flags: {:#x}, fflags: "
                              "{}, data: {}, udata: {} }}",
                              kev.ident, kev.filter, kev.flags, flagstring(kev.fflags),
                              kev.data, kev.udata);
    }
};

namespace loom {
#ifdef __APPLE__

class Kqueue : public Loom<Kqueue> {
public:
    int m_kq_fd;
    std::array<struct kevent, 16> m_changes;
    int m_nchanges;
    struct timespec *m_timeout = nullptr;

public:
    Kqueue() {
        m_kq_fd = kqueue();
        m_nchanges = 0;
        FLUX_ASSERT(m_kq_fd != -1, "Failed to create kqueue");
    }

    ~Kqueue() {
        if (m_kq_fd >= 0) {
            close(m_kq_fd);
        }
    }

    /**
     * This is one epoch of the event loop. Checks all the fds monitored by the event
     * loop and dispatches any events accordingly.
     * @return `true` if the event loop was able to process events, `false` otherwise
     */
    bool epoch() {
        // TODO: 16 can be adjusted based on busy-ness of the event loop. Can even be dynamic
        std::array<struct kevent, 16> events{};

        // changelist: List of events to register with the kernel
        // nchanges: Number of events in the changelist
        // `m_timeout` must be non-empty if we want to also monitor SPSC queues for
        // example `m_timeout` == 0 degrades the performance to a regular poll event loop
        int n = kevent(m_kq_fd, m_changes.data(), m_nchanges, events.data(),
                       events.size(), m_timeout);
        m_nchanges = 0;

        // `kevent` can be interrupted by signals, so we check `errno` global variable.
        if (n < 0 || errno == EINTR)
            return false;

        for (int i = 0; i < n; ++i) {
#ifndef NDEBUG
            std::cout << std::format("Event: {}\n", events[i]);
#endif
            if (events[i].flags & EV_ERROR) {
                printf("Error event\n");
                continue;
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

    void set_timer(int fd, int timer_period, Fiber *fiber) override {
        struct kevent ev {};
        EV_SET(&ev, fd, EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS, 5, nullptr);
        int n = kevent(m_kq_fd, &ev, 1, nullptr, 0, nullptr);
        FLUX_ASSERT(n >= 0, "Failed to set timer");

        // Need to subscribe
        m_subs_by_fd[fd].push_back(fiber);
        m_subcount++;

        // Add to the fiber's list as well
        auto it = std::find(fiber->m_fds.begin(), fiber->m_fds.end(), fd);
        if (it != fiber->m_fds.end()) {
            return;
        }
        fiber->m_fds.push_back(fd);
        // No need to tell kernel to start monitoring because it already is (for timer)
    }

    void handle_sock_op(int fd, loom::Operation op) override {
#ifndef NDEBUG
        FLUX_ASSERT(m_kq_fd >= 0, "Kqueue file descriptor is invalid");
#endif

        throw new std::runtime_error("Not implemented");
        if (m_nchanges >= 10) [[unlikely]] {
            // `changelist` is full, need to flush the changes to the kernel.
            printf("This shouldnt print\n");
            int n = kevent(m_kq_fd, m_changes.data(), m_nchanges, nullptr, 0, nullptr);
            // abstract into a "flush" command
            FLUX_ASSERT(n >= 0, "Failed to flush changes to the kernel");
            m_nchanges = 0;
        }
        // EV_ADD attaches descriptor to kq
        // EV_CLEAR prevents unnecessary signalling
        switch (op) {
        case Operation::Added: {
            // Add RW flags to Operation as well
            EV_SET(&m_changes[m_nchanges++], fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0,
                   nullptr);
            EV_SET(&m_changes[m_nchanges++], fd, EVFILT_WRITE, EV_ADD | EV_CLEAR, 0, 0,
                   nullptr);
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
};
#endif
} /* namespace loom */
