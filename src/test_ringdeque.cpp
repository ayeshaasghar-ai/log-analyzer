#include <iostream>
#include <deque>
#include <vector>
#include <cstdlib>
#include "ringdeque.h"

// Dono deque ka poora content ek jaisa hai?
bool sameContent(const RingDeque& mine, const std::deque<long long>& ref) {
    if (mine.size() != ref.size()) return false;
    for (size_t i = 0; i < ref.size(); i++) {
        if (mine.at(i) != ref[i]) return false;
    }
    return true;
}

bool testBasic() {
    RingDeque d;
    d.push_back(10);
    d.push_back(20);
    d.push_back(30);
    d.push_front(5);
    if (d.size() != 4 || d.front() != 5 || d.back() != 30) return false;
    d.pop_front();
    if (d.front() != 10) return false;
    d.pop_back();
    if (d.back() != 20 || d.size() != 2) return false;
    d.pop_front();
    d.pop_front();
    return d.empty();
}

// Teen daur: bharna, aana-jaana (wrap-around), khaali karna
bool testRandomOperations(int ops, int& mismatches) {
    RingDeque mine;
    std::deque<long long> ref;

    for (int op = 0; op < ops; op++) {
        int phase = op * 3 / ops;                 // 0: bharna, 1: aana-jaana, 2: khaali
        int pushPercent = (phase == 0) ? 75 : (phase == 1 ? 50 : 20);

        bool doPush = ref.empty() || (std::rand() % 100) < pushPercent;
        if (doPush) {
            long long x = std::rand() % 1000;
            if (std::rand() % 2 == 0) {
                mine.push_back(x);
                ref.push_back(x);
            } else {
                mine.push_front(x);
                ref.push_front(x);
            }
        } else {
            if (std::rand() % 2 == 0) {
                if (mine.front() != ref.front()) mismatches++;
                mine.pop_front();
                ref.pop_front();
            } else {
                if (mine.back() != ref.back()) mismatches++;
                mine.pop_back();
                ref.pop_back();
            }
        }

        if (mine.size() != ref.size()) mismatches++;
        if (!ref.empty()) {
            if (mine.front() != ref.front()) mismatches++;
            if (mine.back() != ref.back()) mismatches++;
        }
        if (op % 1000 == 0 && !sameContent(mine, ref)) mismatches++;
    }
    if (!sameContent(mine, ref)) mismatches++;
    return mismatches == 0;
}

// Tool jaisa sliding window: badhte time ke events, har baar ginti compare
bool testSlidingWindow(int events) {
    RingDeque mine;
    std::deque<long long> ref;
    long long t = 0;
    const long long WINDOW = 300;

    for (int i = 0; i < events; i++) {
        t += 1 + std::rand() % 8;
        while (!mine.empty() && mine.front() <= t - WINDOW) mine.pop_front();
        while (!ref.empty() && ref.front() <= t - WINDOW) ref.pop_front();
        if (std::rand() % 100 < 40) {
            mine.push_back(t);
            ref.push_back(t);
        }
        if (mine.size() != ref.size()) return false;
    }
    return sameContent(mine, ref);
}

bool testLargeFill() {
    RingDeque d;
    for (long long i = 0; i < 100000; i++) d.push_back(i);
    if (d.size() != 100000) return false;
    for (long long i = 0; i < 100000; i++) {
        if (d.at(static_cast<size_t>(i)) != i) return false;
    }
    return true;
}

bool testEmptyThrows() {
    RingDeque d;
    int thrown = 0;
    try { d.front(); } catch (const std::out_of_range&) { thrown++; }
    try { d.back(); } catch (const std::out_of_range&) { thrown++; }
    try { d.pop_front(); } catch (const std::out_of_range&) { thrown++; }
    try { d.pop_back(); } catch (const std::out_of_range&) { thrown++; }
    try { d.at(0); } catch (const std::out_of_range&) { thrown++; }
    return thrown == 5;
}

int main() {
    std::srand(11);
    bool ok = true;

    bool basic = testBasic();
    std::cout << "Basic operations: " << (basic ? "OK" : "FAILED") << "\n";
    ok = ok && basic;

    int mismatches = 0;
    const int ops = 300000;
    bool random = testRandomOperations(ops, mismatches);
    std::cout << "Random operations: " << ops << "  mismatches: " << mismatches << "\n";
    ok = ok && random;

    bool window = testSlidingWindow(100000);
    std::cout << "Sliding window simulation (100000 events): "
              << (window ? "OK" : "FAILED") << "\n";
    ok = ok && window;

    bool large = testLargeFill();
    std::cout << "Large fill (100000 items in order): " << (large ? "OK" : "FAILED") << "\n";
    ok = ok && large;

    bool empty = testEmptyThrows();
    std::cout << "Empty-deque check: " << (empty ? "OK" : "FAILED") << "\n";
    ok = ok && empty;

    std::cout << (ok ? "TEST PASSED" : "TEST FAILED") << "\n";
    return ok ? 0 : 1;
}