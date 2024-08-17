#pragma once

#include <cstdint>

namespace loom {
const static uint32_t EVENT_READ = 1 << 0;
const static uint32_t EVENT_WRITE = 1 << 1;
const static uint32_t EVENT_ERROR = 1 << 2;
const static uint32_t EVENT_RWE = EVENT_READ | EVENT_WRITE | EVENT_ERROR;
const static uint32_t EDGE_TRIGGERED = 1 << 3;
const static uint32_t ONE_SHOT = 1 << 4;

struct Event {
    enum Type : uint8_t {
        NA = 0,
        SocketRead = 1,
        SocketWriteable = 2,
        SocketError = 3,
        SocketHangup = 4,
        FileModified = 5,
        Timer = 5,
    };
    Type type;
    int fd;
};
} /* namespace loom */
