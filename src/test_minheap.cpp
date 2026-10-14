#include <iostream>
#include <queue>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <string>
#include "minheap.h"

typedef HeapItem Item;
typedef std::vector<Item> Items;

// a, b se "behtar": zyada ginti, ya barabar ginti par chhota naam
bool isBetter(const Item& a, const Item& b) {
    if (a.second != b.second) return a.second > b.second;
    return a.first < b.first;
}

// a, b se "kharab": heap mein kharab item upar rakhna hai
bool isWorse(const Item& a, const Item& b) {
    return isBetter(b, a);
}

Item randomItem() {
    int id = std::rand() % 300;
    int count = std::rand() % 50;        // chhoti range => bahut barabar ginti
    return Item("KEY_" + std::to_string(id), count);
}

bool testBasicOrder() {
    ItemHeap h(isWorse);
    h.push(Item("B", 5));
    h.push(Item("A", 5));
    h.push(Item("C", 9));
    h.push(Item("D", 1));

    // Kharab se behtar: D(1), B(5), A(5), C(9)
    const char* expected[] = {"D", "B", "A", "C"};
    for (int i = 0; i < 4; i++) {
        if (h.empty() || h.top().first != expected[i]) {
            return false;
        }
        h.pop();
    }
    return h.empty();
}

bool testRandomOperations(int ops, int& mismatches) {
    ItemHeap mine(isWorse);
    std::priority_queue<Item, Items, bool (*)(const Item&, const Item&)> ref(isBetter);

    for (int op = 0; op < ops; op++) {
        bool doPush = ref.empty() || (std::rand() % 100) < 60;
        if (doPush) {
            Item x = randomItem();
            mine.push(x);
            ref.push(x);
        } else {
            if (mine.top() != ref.top()) {
                mismatches++;
            }
            mine.pop();
            ref.pop();
        }
        if (mine.size() != ref.size()) {
            mismatches++;
        }
        if (op % 1000 == 0 && !mine.isValid()) {
            mismatches++;
        }
    }

    while (!ref.empty()) {
        if (mine.empty() || mine.top() != ref.top()) {
            mismatches++;
            break;
        }
        mine.pop();
        ref.pop();
    }
    return mismatches == 0 && mine.empty();
}

// Top K: apna heap (size K) vs poori sort
bool testTopK(size_t n, size_t k) {
    Items items;
    for (size_t i = 0; i < n; i++) {
        items.push_back(Item("KEY_" + std::to_string(i), std::rand() % 1000));
    }

    ItemHeap heap(isWorse);
    for (size_t i = 0; i < items.size(); i++) {
        heap.push(items[i]);
        if (heap.size() > k) {
            heap.pop();
        }
    }
    Items viaHeap;
    while (!heap.empty()) {
        viaHeap.push_back(heap.top());
        heap.pop();
    }
    std::reverse(viaHeap.begin(), viaHeap.end());

    Items viaSort = items;
    std::sort(viaSort.begin(), viaSort.end(), isBetter);
    if (viaSort.size() > k) {
        viaSort.resize(k);
    }
    return viaHeap == viaSort;
}

bool testEmptyThrows() {
    ItemHeap h(isWorse);
    try {
        h.top();
        return false;
    } catch (const std::out_of_range&) {
    }
    try {
        h.pop();
        return false;
    } catch (const std::out_of_range&) {
    }
    return true;
}

int main() {
    std::srand(7);
    bool ok = true;

    bool basic = testBasicOrder();
    std::cout << "Basic order test: " << (basic ? "OK" : "FAILED") << "\n";
    ok = ok && basic;

    int mismatches = 0;
    const int ops = 200000;
    bool random = testRandomOperations(ops, mismatches);
    std::cout << "Random operations: " << ops << "  mismatches: " << mismatches << "\n";
    ok = ok && random;

    size_t ks[] = {1, 5, 50, 1000, 100};
    size_t ns[] = {20000, 20000, 20000, 20000, 10};
    int topkPassed = 0;
    for (int i = 0; i < 5; i++) {
        if (testTopK(ns[i], ks[i])) {
            topkPassed++;
        } else {
            std::cout << "Top-K failed: n=" << ns[i] << " k=" << ks[i] << "\n";
        }
    }
    std::cout << "Top-K tests passed: " << topkPassed << " of 5\n";
    ok = ok && (topkPassed == 5);

    bool empty = testEmptyThrows();
    std::cout << "Empty-heap check: " << (empty ? "OK" : "FAILED") << "\n";
    ok = ok && empty;

    std::cout << (ok ? "TEST PASSED" : "TEST FAILED") << "\n";
    return ok ? 0 : 1;
}