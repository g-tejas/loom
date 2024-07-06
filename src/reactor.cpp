#include "reactor.hpp"

using namespace flux;

Reactor::~Reactor() {}

//!  Subscribe this thread to this file descriptor
void Reactor::subscribe(int fd, Thread *thread) {
  m_subs_by_fd[fd].push_back(thread);
  m_subcount++;
  thread->subscribe(fd);

  // Tell kernel to start monitoring this fd
  handle_sock_op(fd, Operation::Added);
}

//! Remove all subscribers to a particular file descriptor
void Reactor::remove_socket(int fd) {
  for (const auto thread : m_subs_by_fd[fd]) {
    thread->unsubscribe(fd);
    m_subcount--;
  }

  m_subs_by_fd.erase(fd);

  // Tell kernel to stop monitoring this fd
  handle_sock_op(fd, Operation::Removed);
}

//! Remove all subscribers to this thread
void Reactor::remove_thread(Thread *thread) {
  for (int fd : thread->m_fds) {
    auto it = std::find(m_subs_by_fd[fd].begin(), m_subs_by_fd[fd].end(), thread);
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

auto Reactor::active() const -> bool { return m_subcount > 0; }

void Reactor::notify(Event event) {
  printf("Reactor::notify(%d)\n", event.fd);
  std::vector<Thread *> cleanup;
  printf("here\n");
  for (const auto thread : m_subs_by_fd[event.fd]) {
    printf("here2\n");
    if (!thread->resume(&event)) {
      cleanup.push_back(thread);
    }
  }

  printf("finished resuming\n");

  for (const auto thread : cleanup) {
    this->remove_thread(thread);
  }

  printf("finished cleaning up\n");

  if (event.type == Event::Type::SocketHangup || event.type == Event::Type::SocketError) {
    this->remove_socket(event.fd);
  }
}