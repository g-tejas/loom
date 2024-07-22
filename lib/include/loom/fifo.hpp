#pragma once

#include <cassert>
#include <concepts>
#include <type_traits>

namespace loom {
template <typename, typename = std::void_t<>>
struct has_next_pointer : std::false_type {};

template <typename T>
struct has_next_pointer<T, std::void_t<decltype(std::declval<T>().next)>> {
    static constexpr bool value = std::is_pointer_v<decltype(std::declval<T>().next)>;
};

template <typename T>
concept HasNextPointer = has_next_pointer<T>::value;

/// Intrusive Linked List implementation where the type must contain the `next` pointer
template <typename T>
    requires HasNextPointer<T>
class fifo {
  private:
    T *head = nullptr;
    T *tail = nullptr;

  public:
    fifo() = default;

    ~fifo() {
        while (head) {
            T *next = head->next;
            (*head).~T();
            head = next;
        }
    }

    /// TODO: Utilise the trick to copy multiple queue elements in one go where you
    /// virtual map the same queue twice
    bool push(T *elem) {
        // TODO: Replace this with the CHECK_EX macro
        if (elem == nullptr || elem->next != nullptr)
            return false;
        if (head == nullptr) {
            head = elem;
            tail = elem;
        } else {
            tail->next = elem;
            tail = elem;
        }
        return true;
    }

    T *pop() {
        if (head == nullptr)
            return nullptr;
        T *ret = head;
        this->head = head->next;
        ret->next = nullptr;
        if (this->tail == ret)
            this->tail = nullptr;
        return ret;
    }

    T *front() const { return head; }

    T *back() const { return tail; }

    [[nodiscard]] bool empty() const { return front() == nullptr; }

    void clear() {
        while (head) {
            T *next = head->next;
            (*head).~T();
            head = next;
        }
        tail = nullptr;
    }

    fifo(const fifo &) = delete;
    fifo &operator=(const fifo &) = delete;
};
} /* namespace loom */
