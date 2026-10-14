#pragma once
#include <string>
#include <vector>
#include <utility>
#include <stdexcept>

typedef std::pair<std::string, int> HeapItem;

// Apna heap, (naam, ginti) jodiyon ke liye.
// before(a, b) true ho to a ko b se "upar" (top ke qareeb) rakho.
// Isse hum min-heap, max-heap, ya koi bhi rule bana sakte hain.
class ItemHeap {
public:
    typedef bool (*Before)(const HeapItem&, const HeapItem&);

    explicit ItemHeap(Before before) : before_(before) {}

    void push(const HeapItem& item) {
        data_.push_back(item);           // 1) end mein rakho
        siftUp(data_.size() - 1);        // 2) upar tairao
    }

    const HeapItem& top() const {
        if (data_.empty()) {
            throw std::out_of_range("heap is empty");
        }
        return data_[0];
    }

    void pop() {
        if (data_.empty()) {
            throw std::out_of_range("heap is empty");
        }
        data_[0] = data_.back();         // 1) aakhri item upar le aao
        data_.pop_back();
        if (!data_.empty()) {
            siftDown(0);                 // 2) neeche dubao
        }
    }

    size_t size() const { return data_.size(); }
    bool empty() const { return data_.empty(); }

    // Heap rule: koi bachcha apne parent se "upar" nahi hona chahiye
    bool isValid() const {
        for (size_t i = 1; i < data_.size(); i++) {
            if (before_(data_[i], data_[(i - 1) / 2])) {
                return false;
            }
        }
        return true;
    }

private:
    std::vector<HeapItem> data_;
    Before before_;

    void siftUp(size_t i) {
        while (i > 0) {
            size_t parent = (i - 1) / 2;
            if (before_(data_[i], data_[parent])) {
                std::swap(data_[i], data_[parent]);
                i = parent;
            } else {
                break;
            }
        }
    }

    void siftDown(size_t i) {
        size_t n = data_.size();
        while (true) {
            size_t left = 2 * i + 1;
            size_t right = 2 * i + 2;
            size_t best = i;
            if (left < n && before_(data_[left], data_[best])) {
                best = left;
            }
            if (right < n && before_(data_[right], data_[best])) {
                best = right;
            }
            if (best == i) {
                break;
            }
            std::swap(data_[i], data_[best]);
            i = best;
        }
    }
};