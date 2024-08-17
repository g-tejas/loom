#include <array>
#include <loom/io/engine.h>
#include <sys/epoll.h>

namespace loom {

class EpollEngine : public Engine {
private:
    int epoll_fd;
    std::array<struct epoll_event, 16> epoll_events;

public:
    EpollEngine() { this->epoll_fd = epoll_create1(0); }

    ~EpollEngine() override {
        if (this->epoll_fd >= 0) {
            close(this->epoll_fd);
        }
    }

    void set_timer(int fd, int timer_period, Fiber* fiber) override {
        return;
    }

private:
    void handle_sock_op(int fd, Operation op) override {
        return;
    }


};

Engine *new_epoll_engine() { return new EpollEngine(); }
} /* namespace loom */
