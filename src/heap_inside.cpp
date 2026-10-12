#include <iostream>
#include <vector>

// Chhota min-heap (sirf int) jo har kadam dikhata hai.
class MinHeapInt {
public:
    void push(int x) {
        a_.push_back(x);                 // 1) end mein rakho
        std::cout << "push " << x << ": ";
        siftUp(a_.size() - 1);           // 2) upar tairao
        print();
    }

    int pop() {
        int top = a_[0];                 // sabse chhota upar hota hai
        a_[0] = a_.back();               // 1) aakhri item ko upar le aao
        a_.pop_back();
        std::cout << "pop " << top << ": ";
        if (!a_.empty()) {
            siftDown(0);                 // 2) neeche dubao
        }
        print();
        return top;
    }

    bool empty() const { return a_.empty(); }

    // Heap rule check: har bachcha apne parent se bada ya barabar ho
    bool isValid() const {
        for (size_t i = 1; i < a_.size(); i++) {
            if (a_[i] < a_[(i - 1) / 2]) return false;
        }
        return true;
    }

    void print() const {
        std::cout << "array [";
        for (size_t i = 0; i < a_.size(); i++) {
            if (i > 0) std::cout << " ";
            std::cout << a_[i];
        }
        std::cout << "]\n";
    }

    // Index, value, parent, left, right ki table
    void printTable() const {
        std::cout << "index value parent left right\n";
        for (size_t i = 0; i < a_.size(); i++) {
            std::cout << "  " << i << "     " << a_[i] << "     ";
            if (i == 0) std::cout << "-";
            else std::cout << (i - 1) / 2;
            std::cout << "      ";
            size_t l = 2 * i + 1;
            size_t r = 2 * i + 2;
            if (l < a_.size()) std::cout << l; else std::cout << "-";
            std::cout << "     ";
            if (r < a_.size()) std::cout << r; else std::cout << "-";
            std::cout << "\n";
        }
    }

    // Tree ko level by level dikhao
    void printLevels() const {
        size_t levelStart = 0;
        size_t levelSize = 1;
        int level = 0;
        while (levelStart < a_.size()) {
            std::cout << "level " << level << ": ";
            for (size_t i = levelStart; i < levelStart + levelSize && i < a_.size(); i++) {
                std::cout << a_[i] << " ";
            }
            std::cout << "\n";
            levelStart += levelSize;
            levelSize *= 2;
            level++;
        }
    }

private:
    std::vector<int> a_;

    void siftUp(size_t i) {
        while (i > 0) {
            size_t parent = (i - 1) / 2;
            if (a_[i] < a_[parent]) {
                std::cout << "swap " << a_[i] << "<->" << a_[parent] << ", ";
                std::swap(a_[i], a_[parent]);
                i = parent;
            } else {
                break;
            }
        }
    }

    void siftDown(size_t i) {
        size_t n = a_.size();
        while (true) {
            size_t left = 2 * i + 1;
            size_t right = 2 * i + 2;
            size_t smallest = i;
            if (left < n && a_[left] < a_[smallest]) smallest = left;
            if (right < n && a_[right] < a_[smallest]) smallest = right;
            if (smallest == i) break;
            std::cout << "swap " << a_[i] << "<->" << a_[smallest] << ", ";
            std::swap(a_[i], a_[smallest]);
            i = smallest;
        }
    }
};

int main() {
    MinHeapInt h;

    std::cout << "=== Push karte hain: 5 3 8 1 9 2 ===\n";
    int values[] = {5, 3, 8, 1, 9, 2};
    for (int x : values) {
        h.push(x);
    }

    std::cout << "\n=== Index table ===\n";
    h.printTable();

    std::cout << "\n=== Tree (level by level) ===\n";
    h.printLevels();

    std::cout << "\nHeap valid? " << (h.isValid() ? "YES" : "NO") << "\n";

    std::cout << "\n=== Teen baar pop ===\n";
    for (int i = 0; i < 3; i++) {
        h.pop();
    }

    std::cout << "\nHeap valid? " << (h.isValid() ? "YES" : "NO") << "\n";

    std::cout << "\n=== Baaki sab pop ===\n";
    std::vector<int> order;
    while (!h.empty()) {
        order.push_back(h.pop());
    }
    std::cout << "Pop order: ";
    for (size_t i = 0; i < order.size(); i++) {
        std::cout << order[i] << " ";
    }
    std::cout << "\n";
    return 0;
}