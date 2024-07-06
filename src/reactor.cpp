#include "reactor.hpp"

using namespace flux;

Reactor::~Reactor() {}

//!  Subscribe this thread to this file descriptor
void Reactor::subscribe(int fd, Thread thread) {}

//! Remove all subscribers to a particular file descriptor
void Reactor::remove_socket(int fd) {}

//! Remove all subscribers to this thread
void Reactor::remove_thread(Thread *thread) {}

auto Reactor::active() -> bool const {}

void Reactor::notify(Event event) {
  // 1. Get all the thread handles that have subscribed to this fd
  // 2. For each thread handle, resume the thread with the event
  // 3. If the thread is complete after `resume` returns, we need to remove it
  // 4. If the type of the socket event is hang up, we need to remove all the subscribers
  if (event.type == Event::Type::SocketHangup || event.type == Event::Type::SocketError) {
    this->remove_socket(event.fd);
  }
}