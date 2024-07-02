#include "reactor.hpp"

using namespace flux;

Reactor::~Reactor() {}

//! Subscribe this thread to this file descriptor
void Reactor::subscribe(int fd, Thread thread) {}

//! Remove all subscribers to a particular file descriptor
void Reactor::unsubscribe(int fd) {}

//! Remove all subscribers to this thread
void Reactor::unsubscribe(Thread *thread) {}

auto Reactor::active() -> bool const {}
