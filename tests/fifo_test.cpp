#include "flux/fifo.hpp"
#include <catch2/catch_test_macros.hpp>

struct Node {
  int data{};
  Node *next = nullptr;
};

TEST_CASE("Fifo") {
  Fifo<Node> fifo;

  SECTION("Initial state") {
    REQUIRE(fifo.front() == nullptr);
    REQUIRE(fifo.back() == nullptr);
    REQUIRE(fifo.pop() == nullptr);
  }

  SECTION("Single element push and pop") {
    Node n;
    n.data = 1;
    REQUIRE(fifo.push(&n));
    REQUIRE(fifo.front() == &n);
    REQUIRE(fifo.back() == &n);
    REQUIRE(fifo.pop() == &n);
    REQUIRE(fifo.front() == nullptr);
    REQUIRE(fifo.back() == nullptr);
    REQUIRE(fifo.pop() == nullptr);
  }

  SECTION("Multiple elements push and pop") {
    Node n1;
    n1.data = 1;
    Node n2;
    n2.data = 2;
    Node n3;
    n3.data = 3;

    REQUIRE(fifo.push(&n1));
    REQUIRE(fifo.push(&n2));
    REQUIRE(fifo.push(&n3));

    REQUIRE(fifo.front() == &n1);
    REQUIRE(fifo.back() == &n3);

    REQUIRE(fifo.pop() == &n1);
    REQUIRE(fifo.front() == &n2);
    REQUIRE(fifo.back() == &n3);

    REQUIRE(fifo.pop() == &n2);
    REQUIRE(fifo.front() == &n3);
    REQUIRE(fifo.back() == &n3);

    REQUIRE(fifo.pop() == &n3);
    REQUIRE(fifo.front() == nullptr);
    REQUIRE(fifo.back() == nullptr);
    REQUIRE(fifo.pop() == nullptr);
  }

  SECTION("Empty queue") {
    REQUIRE(fifo.empty() == true);
    Node n;
    REQUIRE(fifo.push(&n));
    REQUIRE(fifo.empty() == false);
    fifo.pop();
    REQUIRE(fifo.empty() == true);
  }

  SECTION("Clear function") {
    Node n1;
    n1.data = 1;
    Node n2;
    n2.data = 2;
    REQUIRE(fifo.push(&n1));
    REQUIRE(fifo.push(&n2));

    fifo.clear();
    REQUIRE(fifo.front() == nullptr);
    REQUIRE(fifo.back() == nullptr);
    REQUIRE(fifo.pop() == nullptr);
    REQUIRE(fifo.empty() == true);
  }

  SECTION("Push nullptr (invalid operation)") {
    Node *n = nullptr;
    REQUIRE_FALSE(fifo.push(n));
  }

  SECTION("Handling a large number of elements") {
    const int num_elements = 1000;
    Node nodes[num_elements];

    for (int i = 0; i < num_elements; ++i) {
      nodes[i].data = i;
      REQUIRE(fifo.push(&nodes[i]));
    }

    for (int i = 0; i < num_elements; ++i) {
      Node *n = fifo.pop();
      REQUIRE(n != nullptr);
      REQUIRE(n->data == i);
    }

    REQUIRE(fifo.front() == nullptr);
    REQUIRE(fifo.back() == nullptr);
    REQUIRE(fifo.pop() == nullptr);
  }

  SECTION("Interleaved push and pop operations") {
    Node n1, n2, n3, n4;
    n1.data = 1;
    n2.data = 2;
    n3.data = 3;
    n4.data = 4;

    fifo.push(&n1);
    fifo.push(&n2);
    REQUIRE(fifo.pop() == &n1);
    fifo.push(&n3);
    REQUIRE(fifo.pop() == &n2);
    fifo.push(&n4);
    REQUIRE(fifo.pop() == &n3);
    REQUIRE(fifo.pop() == &n4);
    REQUIRE(fifo.pop() == nullptr);
  }

  SECTION("Reuse elements after popping") {
    Node n1, n2;
    n1.data = 1;
    n2.data = 2;

    fifo.push(&n1);
    fifo.push(&n2);

    Node *popped1 = fifo.pop();
    REQUIRE(popped1 == &n1);
    REQUIRE(fifo.front() == &n2);
    REQUIRE(fifo.back() == &n2);

    Node n3;
    n3.data = 3;
    fifo.push(&n3);

    REQUIRE(fifo.pop() == &n2);
    REQUIRE(fifo.pop() == &n3);
    REQUIRE(fifo.pop() == nullptr);

    fifo.push(popped1);
    REQUIRE(fifo.front() == popped1);
    REQUIRE(fifo.back() == popped1);
    REQUIRE(fifo.pop() == popped1);
    REQUIRE(fifo.pop() == nullptr);
  }
}
