#pragma once

#include <cstdint>
#include <loom/fiber/fiber.h>
#include <optional>

namespace loom {
//! The operations that can be performed on a file descriptor
enum class Operation : uint8_t {
    NIL = 0,
    Added = 1,
    Removed = 2,
    Modified = 3,
    File = 4,
};

const uint32_t EVENT_ENGINE_EPOLL = 1 << 0;
const uint32_t EVENT_ENGINE_KQUEUE = 1 << 1;

#ifdef __APPLE__
const uint32_t EVENT_ENGINE_DEFAULT = EVENT_ENGINE_KQUEUE;
#elif defined(__linux__)
const uint32_t EVENT_ENGINE_DEFAULT = EVENT_ENGINE_EPOLL;
#endif

/**
 * The heart and soul of the Loom library. An abstraction of substrates like `epoll`,
 * `kqueue`. Derived classes need to implement `set_timer`, `handle_sock_op`
 */
class Engine {
public:
    Engine() = default;
    virtual ~Engine() = default;

    Engine(const Engine &) = delete;
    Engine &operator=(const Engine &) = delete;
    Engine(Engine &&) = delete;
    Engine &operator=(Engine &&) = delete;

    auto subscribe(int fd, Fiber *fiber) -> int;
    void remove_socket(int fd);
    void remove_fiber(Fiber *fiber);

    /**
     * @brief Returns true if any subscriptions are active
     * @return True if any subscriptions are active, false otherwise
     */
    [[nodiscard]] auto active() const -> bool;

    virtual void set_timer(int fd, int timer_period, Fiber *fiber) = 0;

protected:
    //! Notify the subscribers of this event about the event. Will cause a context switch.
    void notify(Event event);

    //! Tells the kernel queue of choice to start / stop monitoring the file descriptor
    virtual void handle_sock_op(int fd, Operation op) = 0;

    struct Subscription {
        int fd;
        Fiber *thread;
    };

    std::unordered_map<int, std::vector<Fiber *>> m_subs_by_fd;
    int m_subcount = 0;
};

// We need this funky hack, since we cannot include certain headers on unsupported platforms.
// So we forward declare the methods and then conditionally include the defintions with the build system
#define $LOOM_DECLARE_ENGINE(name) Engine *new_##name##_engine();

$LOOM_DECLARE_ENGINE(epoll);
$LOOM_DECLARE_ENGINE(kqueue);

// Singleton methods
int init(uint32_t event_engine = EVENT_ENGINE_DEFAULT);
std::optional<Engine *> instance() noexcept;
int fini() noexcept;

auto subscribe(int fd, Fiber *fiber) -> int;
auto unsubscribe(int fd) -> int;

} /* namespace loom */
