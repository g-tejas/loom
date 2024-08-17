#include <fmt/color.h>
#include <fmt/printf.h>

#include <loom/fiber/fiber.h>
#include <loom/io/engine.h>
#include <loom/io/event.h>
#include <unordered_map>

namespace loom {

auto Engine::subscribe(int fd, Fiber *fiber) -> int {
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
    return 0;
}

void Engine::remove_socket(int fd) {
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

void Engine::remove_fiber(Fiber *fiber) {
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

auto Engine::active() const -> bool { return true; }

void Engine::notify(Event event) {
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

auto subscribe(int fd, Fiber *fiber) -> int {
    fmt::print(fg(fmt::color::light_blue), "loom::subscribe(fd={}, fiber)\n", fd);
    return (*loom::instance())->subscribe(fd, fiber);
}

thread_local Engine *engine = nullptr;
template <typename Ctor>
inline int _engine_init(Ctor ctor) {
    if (engine) {
        return -1;
    }
    engine = ctor();
    if (!engine) {
        return -1;
    }
    return 0;
}

auto init(const uint32_t event_engine) -> int {
    fmt::print(fmt::emphasis::bold | fg(fmt::color::light_green), "loom::init()\n");
    switch (event_engine) {
#ifdef __linux__
    case EVENT_ENGINE_EPOLL: return new_epoll_engine();
#endif
#ifdef __APPLE__
    case EVENT_ENGINE_KQUEUE: {
        return _engine_init(&new_kqueue_engine);
    }
#endif
    default: return -1;
    }
}

auto instance() noexcept -> std::optional<Engine *> {
    if (!engine) {
        fmt::print(fmt::emphasis::bold | fg(fmt::color::red),
                   "loom::instance() - engine is not initialized\n");
        return std::nullopt;
    }
    return engine;
}

int fini() noexcept {
    fmt::print(fmt::emphasis::bold | fg(fmt::color::light_green), "loom::fini()\n");
    if (!engine) {
        return -1;
    }
    delete engine;
    engine = nullptr;
    return 0;
}
} // namespace loom
