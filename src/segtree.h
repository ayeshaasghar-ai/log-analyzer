#pragma once
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <limits>
#include <cstddef>

// Apna segment tree (sum aur max dono), long long values ke liye.
// Node 1 root hai (poori range). Node i ke bachche: 2*i (left) aur 2*i+1 (right).
// Saari ranges [l, r] hain, dono sire shamil.
class SegTree {
public:
    SegTree() : n_(0) {}

    explicit SegTree(const std::vector<long long>& a) : n_(0) {
        build(a);
    }

    // Tree banao: O(n)
    void build(const std::vector<long long>& a) {
        n_ = static_cast<int>(a.size());
        sum_.assign(4 * a.size() + 4, 0);
        mx_.assign(4 * a.size() + 4, kNothing);
        if (n_ > 0) {
            buildNode(1, 0, n_ - 1, a);
        }
    }

    size_t size() const { return static_cast<size_t>(n_); }

    // l se r tak ka jod: O(log n)
    long long sum(size_t l, size_t r) const {
        checkRange(l, r);
        return sumNode(1, 0, n_ - 1, static_cast<int>(l), static_cast<int>(r));
    }

    // l se r tak ka sabse bada value: O(log n)
    long long maxValue(size_t l, size_t r) const {
        checkRange(l, r);
        return maxNode(1, 0, n_ - 1, static_cast<int>(l), static_cast<int>(r));
    }

    // a[i] = value: O(log n)
    void set(size_t i, long long value) {
        checkIndex(i);
        setNode(1, 0, n_ - 1, static_cast<int>(i), value);
    }

    // a[i] += delta: O(log n)
    void add(size_t i, long long delta) {
        set(i, sum(i, i) + delta);
    }

private:
    // "Kuch nahi" ka max: itna chhota ke koi asli value isse chhoti nahi ho sakti
    static constexpr long long kNothing = std::numeric_limits<long long>::lowest();

    int n_;
    std::vector<long long> sum_;
    std::vector<long long> mx_;

    void checkIndex(size_t i) const {
        if (i >= static_cast<size_t>(n_)) {
            throw std::out_of_range("index out of range");
        }
    }

    void checkRange(size_t l, size_t r) const {
        if (l > r || r >= static_cast<size_t>(n_)) {
            throw std::out_of_range("bad range");
        }
    }

    void pull(int node) {
        sum_[node] = sum_[2 * node] + sum_[2 * node + 1];
        mx_[node] = std::max(mx_[2 * node], mx_[2 * node + 1]);
    }

    void buildNode(int node, int l, int r, const std::vector<long long>& a) {
        if (l == r) {
            sum_[node] = a[l];
            mx_[node] = a[l];
            return;
        }
        int mid = (l + r) / 2;
        buildNode(2 * node, l, mid, a);
        buildNode(2 * node + 1, mid + 1, r, a);
        pull(node);
    }

    long long sumNode(int node, int l, int r, int ql, int qr) const {
        if (qr < l || r < ql) return 0;                  // bilkul bahar
        if (ql <= l && r <= qr) return sum_[node];       // poora andar
        int mid = (l + r) / 2;
        return sumNode(2 * node, l, mid, ql, qr)
             + sumNode(2 * node + 1, mid + 1, r, ql, qr);
    }

    long long maxNode(int node, int l, int r, int ql, int qr) const {
        if (qr < l || r < ql) return kNothing;           // bilkul bahar
        if (ql <= l && r <= qr) return mx_[node];        // poora andar
        int mid = (l + r) / 2;
        return std::max(maxNode(2 * node, l, mid, ql, qr),
                        maxNode(2 * node + 1, mid + 1, r, ql, qr));
    }

    void setNode(int node, int l, int r, int idx, long long value) {
        if (l == r) {
            sum_[node] = value;
            mx_[node] = value;
            return;
        }
        int mid = (l + r) / 2;
        if (idx <= mid) setNode(2 * node, l, mid, idx, value);
        else setNode(2 * node + 1, mid + 1, r, idx, value);
        pull(node);                                      // wapas aate hue parent dobara jodo
    }
};