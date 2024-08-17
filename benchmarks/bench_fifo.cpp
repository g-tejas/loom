#include <benchmark/benchmark.h>
#include <forward_list>
#include <loom/loom.h>

struct Node {
    int value;
    Node *next;
};

static void BM_LoomFifoPush(benchmark::State &state) {
    loom::fifo<Node> fifo;
    for (auto _ : state) {
        Node *node = new Node;
        node->value = 42;
        fifo.push(node);
    }
}

static void BM_LoomFifoPop(benchmark::State &state) {
    loom::fifo<Node> fifo;
    for (auto _ : state) {
        Node *node = new Node;
        node->value = 42;
        fifo.push(node);
        fifo.pop();
    }
}

static void BM_StdForwardListPush(benchmark::State &state) {
    std::forward_list<Node> list;
    for (auto _ : state) {
        Node *node = new Node;
        node->value = 42;
        list.push_front(*node);
    }
}

static void BM_StdForwardListPop(benchmark::State &state) {
    std::forward_list<Node> list;
    for (auto _ : state) {
        Node *node = new Node;
        node->value = 42;
        list.push_front(*node);
        list.pop_front();
    }
}

BENCHMARK(BM_LoomFifoPush);
BENCHMARK(BM_LoomFifoPop);
BENCHMARK(BM_StdForwardListPush);
BENCHMARK(BM_StdForwardListPop);

BENCHMARK_MAIN();
