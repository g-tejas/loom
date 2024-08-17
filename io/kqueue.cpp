#pragma once

#include "loom/common/utils.h"
#include "loom/io/engine.h"

#include <fmt/base.h>
#include <fmt/format.h>
#include <sstream>
#include <string>
#ifdef __APPLE__
#include <sys/event.h>

template <>
struct fmt::formatter<struct kevent> {
    constexpr auto parse(auto &ctx) { return ctx.begin(); }

    auto format(const struct kevent &kev, auto &ctx) const {
        return fmt::format_to(ctx.out(),
                              "kevent{{ ident: {}, filter: {}, flags: {:#x}, fflags: "
                              "{}, data: {}, udata: {} }}",
                              kev.ident, filter_to_string(kev.filter), kev.flags,
                              fflag_to_string(kev.fflags), kev.data, kev.udata);
    }

private:
    static [[nodiscard]] std::string fflag_to_string(uint32_t fflags) {
        std::ostringstream ret;
        bool first = true;

        for (const auto &[flag, name] : flag_names) {
            if (fflags & flag) {
                if (!first) {
                    ret << "|";
                }
                ret << name;
                first = false;
            }
        }

        return ret.str();
    }

    static std::string_view filter_to_string(int16_t filter) {
        auto it = filter_names.find(filter);
        if (it != filter_names.end()) {
            return it->second;
        } else {
            return "UNKNOWN_FILTER";
        }
    }

    inline static const std::unordered_map<int16_t, const char *> filter_names = {
        {EVFILT_READ, "EVFILT_READ"},
        {EVFILT_WRITE, "EVFILT_WRITE"},
        {EVFILT_AIO, "EVFILT_AIO"},
        {EVFILT_VNODE, "EVFILT_VNODE"},
        {EVFILT_PROC, "EVFILT_PROC"},
        {EVFILT_SIGNAL, "EVFILT_SIGNAL"},
        {EVFILT_TIMER, "EVFILT_TIMER"},
        {EVFILT_MACHPORT, "EVFILT_MACHPORT"},
        {EVFILT_FS, "EVFILT_FS"},
        {EVFILT_USER, "EVFILT_USER"},
        {EVFILT_VM, "EVFILT_VM"},
        {EVFILT_EXCEPT, "EVFILT_EXCEPT"},
        {EVFILT_SYSCOUNT, "EVFILT_SYSCOUNT"},
        {EVFILT_THREADMARKER, "EVFILT_THREADMARKER"}};

    constexpr static std::pair<uint32_t, const char *> flag_names[] = {
        {NOTE_TRIGGER, "NOTE_TRIGGER"},
        {NOTE_FFNOP, "NOTE_FFNOP"},
        {NOTE_FFAND, "NOTE_FFAND"},
        {NOTE_FFOR, "NOTE_FFOR"},
        {NOTE_FFCOPY, "NOTE_FFCOPY"},
        {NOTE_FFCTRLMASK, "NOTE_FFCTRLMASK"},
        {NOTE_FFLAGSMASK, "NOTE_FFLAGSMASK"},
        {NOTE_LOWAT, "NOTE_LOWAT"},
        {NOTE_OOB, "NOTE_OOB"},
        {NOTE_DELETE, "NOTE_DELETE"},
        {NOTE_WRITE, "NOTE_WRITE"},
        {NOTE_EXTEND, "NOTE_EXTEND"},
        {NOTE_ATTRIB, "NOTE_ATTRIB"},
        {NOTE_LINK, "NOTE_LINK"},
        {NOTE_RENAME, "NOTE_RENAME"},
        {NOTE_REVOKE, "NOTE_REVOKE"},
        {NOTE_NONE, "NOTE_NONE"},
        {NOTE_FUNLOCK, "NOTE_FUNLOCK"},
        {NOTE_LEASE_DOWNGRADE, "NOTE_LEASE_DOWNGRADE"},
        {NOTE_LEASE_RELEASE, "NOTE_LEASE_RELEASE"},
        {NOTE_EXIT, "NOTE_EXIT"},
        {NOTE_FORK, "NOTE_FORK"},
        {NOTE_EXEC, "NOTE_EXEC"},
        {NOTE_SIGNAL, "NOTE_SIGNAL"},
        {NOTE_EXITSTATUS, "NOTE_EXITSTATUS"},
        {NOTE_EXIT_DETAIL, "NOTE_EXIT_DETAIL"},
        {NOTE_PDATAMASK, "NOTE_PDATAMASK"},
        {NOTE_EXIT_DETAIL_MASK, "NOTE_EXIT_DETAIL_MASK"},
        {NOTE_EXIT_DECRYPTFAIL, "NOTE_EXIT_DECRYPTFAIL"},
        {NOTE_EXIT_MEMORY, "NOTE_EXIT_MEMORY"},
        {NOTE_EXIT_CSERROR, "NOTE_EXIT_CSERROR"},
        {NOTE_VM_PRESSURE, "NOTE_VM_PRESSURE"},
        {NOTE_VM_PRESSURE_TERMINATE, "NOTE_VM_PRESSURE_TERMINATE"},
        {NOTE_VM_PRESSURE_SUDDEN_TERMINATE, "NOTE_VM_PRESSURE_SUDDEN_TERMINATE"},
        {NOTE_VM_ERROR, "NOTE_VM_ERROR"},
        {NOTE_SECONDS, "NOTE_SECONDS"},
        {NOTE_USECONDS, "NOTE_USECONDS"},
        {NOTE_NSECONDS, "NOTE_NSECONDS"},
        {NOTE_ABSOLUTE, "NOTE_ABSOLUTE"},
        {NOTE_LEEWAY, "NOTE_LEEWAY"},
        {NOTE_CRITICAL, "NOTE_CRITICAL"},
        {NOTE_BACKGROUND, "NOTE_BACKGROUND"},
        {NOTE_MACH_CONTINUOUS_TIME, "NOTE_MACH_CONTINUOUS_TIME"},
        {NOTE_MACHTIME, "NOTE_MACHTIME"},
        {NOTE_TRACK, "NOTE_TRACK"},
        {NOTE_TRACKERR, "NOTE_TRACKERR"},
        {NOTE_CHILD, "NOTE_CHILD"}};
};

namespace loom {
class KqueueEngine : public Engine {
private:
    int m_kq_fd;
    std::array<struct kevent, 16> m_changes;
    int m_nchanges;
    struct timespec m_timeout {};

public:
    KqueueEngine() {
        m_kq_fd = kqueue();
        m_nchanges = 0;
        m_timeout.tv_sec = 0;
        m_timeout.tv_nsec = 500000000;
        LOOM_ASSERT(m_kq_fd != -1, "Failed to create kqueue");
    }

    ~KqueueEngine() override {
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
                       events.size(), &m_timeout);
        m_nchanges = 0;

        // `kevent` can be interrupted by signals, so we check `errno` global variable.
        if (n < 0 || errno == EINTR)
            return false;

        for (int i = 0; i < n; ++i) {
#ifndef NDEBUG
            fmt::print("{}\n", events[i]);
#endif
            if (events[i].flags & EV_ERROR) {
                printf("Error event\n");
                continue;
            }
            if (events[i].filter == EVFILT_READ) {
                // Not just socket reads but reads for files too
                this->notify(Event{.type = Event::Type::SocketRead,
                                   .fd = static_cast<int>(events[i].ident)});
            } else if (events[i].filter == EVFILT_WRITE) {
                this->notify(Event{.type = Event::Type::SocketWriteable,
                                   .fd = static_cast<int>(events[i].ident)});
            } else if (events[i].filter == EVFILT_TIMER) {
                this->notify(Event{.type = Event::Type::Timer,
                                   .fd = static_cast<int>(events[i].ident)});
            } else if (events[i].filter == EVFILT_VNODE) {
                this->notify(Event{.type = Event::Type::FileModified,
                                   .fd = static_cast<int>(events[i].ident)});
            } else {
                printf("Unknown event\n");
            }
        }

        // Reset the timeout. In case of a signal interruption the values might change
        m_timeout.tv_sec = 0;
        m_timeout.tv_nsec = 500000000;

        return true;
    }

    void set_timer(int fd, int timer_period, Fiber *fiber) override {
        struct kevent ev {};
        EV_SET(&ev, fd, EVFILT_TIMER, EV_ADD | EV_ENABLE, NOTE_SECONDS, timer_period,
               nullptr);
        int n = kevent(m_kq_fd, &ev, 1, nullptr, 0, nullptr);
        LOOM_ASSERT(n >= 0, "Failed to set timer");

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

    void handle_sock_op(int fd, Operation op) override {
#ifndef NDEBUG
        LOOM_ASSERT(m_kq_fd >= 0, "Kqueue file descriptor is invalid");
#endif

        if (m_nchanges >= 10) [[unlikely]] {
            // `changelist` is full, need to flush the changes to the kernel.
            int n = kevent(m_kq_fd, m_changes.data(), m_nchanges, nullptr, 0, nullptr);
            // abstract into a "flush" command
            LOOM_ASSERT(n >= 0, "Failed to flush changes to the kernel");
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
        case Operation::File:
            EV_SET(&m_changes[m_nchanges++], fd, EVFILT_VNODE, EV_ADD | EV_CLEAR,
                   NOTE_DELETE | NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_LINK |
                       NOTE_RENAME | NOTE_REVOKE,
                   0, nullptr);
            EV_SET(&m_changes[m_nchanges++], fd, EVFILT_READ, EV_ADD | EV_CLEAR, 0, 0,
                   nullptr);
            break;
        case Operation::Modified: [[fallthrough]];
        case Operation::NIL: break;
        }
    }
};

Engine *new_kqueue_engine() { return new KqueueEngine(); }
} /* namespace loom */
#endif
