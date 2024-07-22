#pragma once

#include <cstdint>

namespace loom {
struct Event {
    enum Type : uint8_t {
        NA = 0,
        SocketRead = 1,
        SocketWriteable = 2,
        SocketError = 3,
        SocketHangup = 4,
        Timer = 5,
    };
    Type type;
    int fd;
};
} /* namespace loom */
