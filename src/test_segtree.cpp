#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include "segtree.h"

// Brute force: l se r tak ka sum
long long bruteSum(const std::vector<long long>& a, size_t l, size_t r) {
    long long s = 0;
    for (size_t i = l; i <= r; i++) s += a[i];
    return s;
}

// Brute force: l se r tak ka max
long long bruteMax(const std::vector<long long>& a, size_t l, size_t r) {
    long long best = a[l];
    for (size_t i = l + 1; i <= r; i++) best = std::max(best, a[i]);
    return best;
}

bool testBasic() {
    std::vector<long long> a = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    SegTree t(a);
    if (t.size() != 10) return false;
    if (t.sum(2, 5) != 19 || t.maxValue(2, 5) != 9) return false;
    if (t.sum(0, 9) != 39 || t.maxValue(0, 9) != 9) return false;
    if (t.sum(7, 7) != 6) return false;
    t.set(4, 10);
    if (t.sum(2, 5) != 24 || t.maxValue(2, 5) != 10 || t.sum(0, 9) != 44) return false;
    t.add(4, -7);                       // 10 - 7 = 3
    return t.sum(4, 4) == 3 && t.sum(0, 9) != 44 && t.sum(0, 9) == 37;
}

// Alag alag sizes (power of two aur uske bina) par saari ranges, negative values ke saath
bool testAllRanges() {
    int sizes[] = {1, 2, 3, 5, 7, 8, 10, 13, 16, 31, 64, 100};
    for (int s = 0; s < 12; s++) {
        int n = sizes[s];
        std::vector<long long> a(n);
        for (int i = 0; i < n; i++) a[i] = (std::rand() % 101) - 50;     // -50 se 50
        SegTree t(a);
        for (int l = 0; l < n; l++) {
            for (int r = l; r < n; r++) {
                if (t.sum(l, r) != bruteSum(a, l, r)) return false;
                if (t.maxValue(l, r) != bruteMax(a, l, r)) return false;
            }
        }
    }
    return true;
}

// Random set / add / sum / max, har baar brute force se milan
bool testRandomOperations(int ops, int& mismatches) {
    const int n = 1000;
    std::vector<long long> a(n);
    for (int i = 0; i < n; i++) a[i] = (std::rand() % 2001) - 1000;
    SegTree t(a);

    for (int op = 0; op < ops; op++) {
        int kind = std::rand() % 4;
        size_t i = static_cast<size_t>(std::rand() % n);
        size_t j = static_cast<size_t>(std::rand() % n);
        size_t l = std::min(i, j);
        size_t r = std::max(i, j);
        if (kind == 0) {
            long long v = (std::rand() % 2001) - 1000;
            t.set(i, v);
            a[i] = v;
        } else if (kind == 1) {
            long long d = (std::rand() % 201) - 100;
            t.add(i, d);
            a[i] += d;
        } else if (kind == 2) {
            if (t.sum(l, r) != bruteSum(a, l, r)) mismatches++;
        } else {
            if (t.maxValue(l, r) != bruteMax(a, l, r)) mismatches++;
        }
    }
    return mismatches == 0;
}

// Bade numbers (overflow ka dar): long long mein sahi jod
bool testBigValues() {
    std::vector<long long> a(1000, 4000000000000000LL);     // 4e15 har item
    SegTree t(a);
    return t.sum(0, 999) == 4000000000000000000LL && t.maxValue(0, 999) == 4000000000000000LL;
}

bool testEdgeCases() {
    SegTree empty;
    if (empty.size() != 0) return false;
    int thrown = 0;
    try { empty.sum(0, 0); } catch (const std::out_of_range&) { thrown++; }
    try { empty.set(0, 1); } catch (const std::out_of_range&) { thrown++; }

    std::vector<long long> a = {7, 8, 9};
    SegTree t(a);
    try { t.sum(2, 1); } catch (const std::out_of_range&) { thrown++; }        // l > r
    try { t.maxValue(0, 3); } catch (const std::out_of_range&) { thrown++; }   // r bahar
    try { t.add(3, 1); } catch (const std::out_of_range&) { thrown++; }

    SegTree one(std::vector<long long>(1, 42));
    bool oneOk = (one.sum(0, 0) == 42 && one.maxValue(0, 0) == 42);
    return thrown == 5 && oneOk;
}

int main() {
    std::srand(5);
    bool ok = true;

    bool basic = testBasic();
    std::cout << "Basic operations: " << (basic ? "OK" : "FAILED") << "\n";
    ok = ok && basic;

    bool all = testAllRanges();
    std::cout << "All ranges, sizes 1 to 100 (12 sizes, negative values too): "
              << (all ? "OK" : "FAILED") << "\n";
    ok = ok && all;

    int mismatches = 0;
    const int ops = 200000;
    bool random = testRandomOperations(ops, mismatches);
    std::cout << "Random operations: " << ops << "  mismatches: " << mismatches << "\n";
    ok = ok && random;

    bool big = testBigValues();
    std::cout << "Big values (sum of 4e18): " << (big ? "OK" : "FAILED") << "\n";
    ok = ok && big;

    bool edge = testEdgeCases();
    std::cout << "Edge cases and errors: " << (edge ? "OK" : "FAILED") << "\n";
    ok = ok && edge;

    std::cout << (ok ? "TEST PASSED" : "TEST FAILED") << "\n";
    return ok ? 0 : 1;
}