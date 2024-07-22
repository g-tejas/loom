#pragma once

#include <sys/event.h>

#include "loom/event.hpp"
#include "loom/fiber.hpp"
#include <unordered_map>

namespace loom {
//! The operations that can be performed on a file descriptor
enum class Operation : uint8_t {
    NIL = 0,
    Added = 1,
    Removed = 2,
    Modified = 3,
};

//! The heart and soul of the loom library. Handles fiber dispatch
class Loom {
public:
    Loom();

    ~Loom();

    Loom(const Loom &) = delete;
    Loom &operator=(const Loom &) = delete;

    auto subscribe(int fd, Fiber *fiber) -> int;

    void remove_socket(int fd);
    void remove_fiber(Fiber *fiber);

    /**
     * @brief Returns true if any subscriptions are active
     * @return True if any subscriptions are active, false otherwise
     */
    [[nodiscard]] auto active() const -> bool;

    //! Notify the subscribers of this event about the event. Will cause a context switch.
    void notify(Event event);

    //! Runs the event loop for given time or until there are no subscriptions left,
    //! whichever comes first. Look at TigerBeetle's implementation
    auto run_for_ns(uint64_t ns) const -> bool;

protected:
    struct Subscription {
        int fd;
        Fiber *thread;
    };

    std::unordered_map<int, std::vector<Fiber *>> m_subs_by_fd;
    int m_subcount{};

    //! Tells the kernel queue of choice to start / stop monitoring the file descriptor
    void handle_sock_op(int fd, Operation op);

    bool epoch();

    void set_timer(int id, int timer_period);

private:
#ifdef LOOM_BACKEND_KQUEUE
    int m_kqueue_fd;
    std::array<struct kevent, 16> m_event{};
    std::vector<struct kevent> m_changes;
    //! How long to block waiting for events. `nullptr` means block indefinitely
    timespec *m_timeout;
#endif
};

auto Loom::subscribe(int fd, Fiber *fiber) -> int {
    // Add to the looms
    m_subs_by_fd[fd].push_back(fiber);
    m_subcount++;

    // Add to the fiber's list as well
    auto it = std::find(fiber->m_fds.begin(), fiber->m_fds.end(), fd);
    if (it != fiber->m_fds.end()) {
        return 1;
    }
    fiber->m_fds.push_back(fd);

    // Tell kernel to start monitoring this fd
    handle_sock_op(fd, Operation::Added);
}

void Loom::remove_socket(int fd) {
    for (const auto fiber : m_subs_by_fd[fd]) {
        auto it = std::find(fiber->m_fds.begin(), fiber->m_fds.end(), fd);
        if (it != fiber->m_fds.end()) {
            fiber->m_fds.erase(it);
            m_subcount--;
        }
    }

    m_subs_by_fd.erase(fd);

    // Tell kernel to stop monitoring this fd
    handle_sock_op(fd, Operation::Removed);
}

void Loom::remove_fiber(Fiber *fiber) {
    for (int fd : fiber->m_fds) {
        auto it = std::find(m_subs_by_fd[fd].begin(), m_subs_by_fd[fd].end(), fiber);
        if (it != m_subs_by_fd[fd].end()) {
            m_subs_by_fd[fd].erase(it);
            m_subcount--;
        }

        // if the fd hits zero, remove the socket
        if (m_subs_by_fd[fd].empty()) {
            handle_sock_op(fd, Operation::Removed);
            m_subs_by_fd.erase(fd);
        }
    }
}

auto Loom::active() const -> bool { return m_subcount > 0; }

void Loom::notify(Event event) {
    std::vector<Fiber *> cleanup;
    for (const auto fiber : m_subs_by_fd[event.fd]) {
        if (!fiber->resume(&event)) {
            cleanup.push_back(fiber);
        }
    }

    for (const auto fiber : cleanup) {
        this->remove_fiber(fiber);
    }

    if (event.type == Event::Type::SocketHangup ||
        event.type == Event::Type::SocketError) {
        this->remove_socket(event.fd);
    }
}

auto Loom::run_for_ns(uint64_t ns) const -> bool { return true; }

#ifdef LOOM_BACKEND_KQUEUE
#include "loom/backends/kqueue.hpp"
#endif

#ifdef LOOM_BACKEND_EPOLL
#include "loom/backends/epoll.hpp"
#endif

} // namespace loom
