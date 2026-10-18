#include <iostream>
#include <deque>
#include <vector>

// Chhota ring buffer deque (sirf int) jo har kadam dikhata hai.
// Data ek fixed array mein rehta hai. `head` pehle item ki jagah batata hai,
// aur `count` batata hai kitne items hain. Array ke end par pahunch kar
// wapas shuru par ghoom jaate hain (modulo), isliye "ring" (gol) kehte hain.
class RingDemo {
public:
    explicit RingDemo(size_t capacity) : buf_(capacity, 0), head_(0), count_(0) {}

    void pushBack(int x) {
        growIfFull();
        size_t pos = (head_ + count_) % buf_.size();   // pehle item se count aage
        buf_[pos] = x;
        count_++;
        std::cout << "push_back " << x << " at slot " << pos << ": ";
        print();
    }

    void pushFront(int x) {
        growIfFull();
        head_ = (head_ + buf_.size() - 1) % buf_.size();   // head ek peeche
        buf_[head_] = x;
        count_++;
        std::cout << "push_front " << x << " at slot " << head_ << ": ";
        print();
    }

    void popFront() {
        std::cout << "pop_front " << buf_[head_] << ": ";
        head_ = (head_ + 1) % buf_.size();                 // head ek aage
        count_--;
        print();
    }

    void popBack() {
        size_t pos = (head_ + count_ - 1) % buf_.size();
        std::cout << "pop_back " << buf_[pos] << ": ";
        count_--;
        print();
    }

    int front() const { return buf_[head_]; }
    size_t size() const { return count_; }
    bool empty() const { return count_ == 0; }

    // Raw array, head, count, aur sahi (logical) order dikhao
    void print() const {
        std::cout << "slots [";
        for (size_t i = 0; i < buf_.size(); i++) {
            if (i > 0) std::cout << " ";
            if (isUsed(i)) std::cout << buf_[i];
            else std::cout << ".";
        }
        std::cout << "] head=" << head_ << " count=" << count_
                  << " cap=" << buf_.size() << " | order:";
        for (size_t i = 0; i < count_; i++) {
            std::cout << " " << buf_[(head_ + i) % buf_.size()];
        }
        std::cout << "\n";
    }

private:
    std::vector<int> buf_;
    size_t head_;
    size_t count_;

    // Slot i abhi kisi item ka hai ya khaali?
    bool isUsed(size_t i) const {
        size_t offset = (i + buf_.size() - head_) % buf_.size();
        return offset < count_;
    }

    // Array bhar jaye to double karo aur items ko sahi order mein shuru se rakho
    void growIfFull() {
        if (count_ < buf_.size()) return;
        size_t oldCap = buf_.size();
        std::vector<int> bigger(oldCap * 2, 0);
        for (size_t i = 0; i < count_; i++) {
            bigger[i] = buf_[(head_ + i) % oldCap];
        }
        buf_.swap(bigger);
        head_ = 0;
        std::cout << "(grow: capacity " << oldCap << " -> " << buf_.size() << ") ";
    }
};

int main() {
    // ---------- Part 1: wrap-around aur grow ----------
    std::cout << "=== Part 1: push_back, pop_front, wrap-around, grow ===\n";
    RingDemo r(4);
    r.pushBack(10);
    r.pushBack(20);
    r.pushBack(30);
    r.popFront();
    r.pushBack(40);
    r.pushBack(50);   // array ke end se ghoom kar slot 0 par jayega
    r.pushBack(60);   // ab bhar chuka hai: grow hoga

    // ---------- Part 2: push_front aur pop_back ----------
    std::cout << "\n=== Part 2: push_front aur pop_back ===\n";
    r.pushFront(5);   // head peeche ghoom kar aakhri slot par
    r.popBack();

    // ---------- Part 3: sliding window, ring ka use ----------
    std::cout << "\n=== Part 3: sliding window (pichhle 10 second) ===\n";
    std::vector<int> events = {1, 3, 4, 8, 12, 13, 25};
    const int WINDOW = 10;

    RingDemo window(4);
    std::deque<int> reference;                 // std::deque se jawab milane ke liye
    bool same = true;

    for (size_t i = 0; i < events.size(); i++) {
        int t = events[i];

        while (!window.empty() && window.front() <= t - WINDOW) {
            window.popFront();
        }
        while (!reference.empty() && reference.front() <= t - WINDOW) {
            reference.pop_front();
        }

        window.pushBack(t);
        reference.push_back(t);

        std::cout << "  -> t=" << t << " count " << window.size() << "\n";
        if (window.size() != reference.size()) {
            same = false;
        }
    }
    std::cout << "\nRing buffer matches std::deque: " << (same ? "YES" : "NO") << "\n";
    return 0;
}