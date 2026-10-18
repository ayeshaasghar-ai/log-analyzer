#pragma once
#include <vector>
#include <stdexcept>
#include <cstddef>

// Apna deque (ring buffer), long long ke liye.
// Function ke naam std::deque jaise rakhe hain, taaki baad mein
// std::deque ki jagah seedha lagaya ja sake.
//
// Data ek array (buf_) mein rehta hai. head_ pehle item ki jagah batata hai,
// count_ batata hai kitne items hain. Item number i (aage se) ka asli slot:
//     (head_ + i) % buf_.size()
class RingDeque {
public:
    RingDeque() : buf_(8, 0), head_(0), count_(0) {}

    void push_back(long long x) {
        growIfFull();
        buf_[(head_ + count_) % buf_.size()] = x;
        count_++;
    }

    void push_front(long long x) {
        growIfFull();
        head_ = (head_ + buf_.size() - 1) % buf_.size();
        buf_[head_] = x;
        count_++;
    }

    void pop_front() {
        if (count_ == 0) {
            throw std::out_of_range("deque is empty");
        }
        head_ = (head_ + 1) % buf_.size();
        count_--;
    }

    void pop_back() {
        if (count_ == 0) {
            throw std::out_of_range("deque is empty");
        }
        count_--;
    }

    long long front() const {
        if (count_ == 0) {
            throw std::out_of_range("deque is empty");
        }
        return buf_[head_];
    }

    long long back() const {
        if (count_ == 0) {
            throw std::out_of_range("deque is empty");
        }
        return buf_[(head_ + count_ - 1) % buf_.size()];
    }

    // Aage se i-wa item (0 = front)
    long long at(size_t i) const {
        if (i >= count_) {
            throw std::out_of_range("index out of range");
        }
        return buf_[(head_ + i) % buf_.size()];
    }

    size_t size() const { return count_; }
    bool empty() const { return count_ == 0; }

private:
    std::vector<long long> buf_;
    size_t head_;
    size_t count_;

    // Bhar jaye to double karo, items ko sahi order mein shuru se rakho
    void growIfFull() {
        if (count_ < buf_.size()) {
            return;
        }
        size_t oldCap = buf_.size();
        std::vector<long long> bigger(oldCap * 2, 0);
        for (size_t i = 0; i < count_; i++) {
            bigger[i] = buf_[(head_ + i) % oldCap];
        }
        buf_.swap(bigger);
        head_ = 0;
    }
};