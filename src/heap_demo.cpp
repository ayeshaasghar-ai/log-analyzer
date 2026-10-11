#include <iostream>
#include <queue>
#include <vector>
#include <functional>

int main() {
    int nums[] = {5, 1, 8, 3, 9, 2};

    // 1) Max-heap: sabse bada upar
    std::priority_queue<int> maxHeap;
    for (int x : nums) {
        maxHeap.push(x);
    }
    std::cout << "Max-heap pop order: ";
    while (!maxHeap.empty()) {
        std::cout << maxHeap.top() << " ";
        maxHeap.pop();
    }
    std::cout << "\n";

    // 2) Min-heap: sabse chhota upar
    std::priority_queue<int, std::vector<int>, std::greater<int>> minHeap;
    for (int x : nums) {
        minHeap.push(x);
    }
    std::cout << "Min-heap pop order: ";
    while (!minHeap.empty()) {
        std::cout << minHeap.top() << " ";
        minHeap.pop();
    }
    std::cout << "\n\n";

    // 3) Top 3 bade numbers: min-heap jo kabhi 3 se zyada nahi rakhta
    int data[] = {7, 2, 9, 4, 11, 5, 8};
    const size_t K = 3;
    std::priority_queue<int, std::vector<int>, std::greater<int>> h;

    std::cout << "Top " << K << " of: 7 2 9 4 11 5 8\n";
    for (int x : data) {
        h.push(x);
        std::cout << "push " << x << " -> size " << h.size();
        if (h.size() > K) {
            std::cout << " -> too big, remove smallest (" << h.top() << ")";
            h.pop();
        }
        std::cout << " | smallest kept: " << h.top() << "\n";
    }

    std::vector<int> out;
    while (!h.empty()) {
        out.push_back(h.top());
        h.pop();
    }
    std::cout << "Top " << K << " (biggest first): ";
    for (size_t i = out.size(); i > 0; i--) {
        std::cout << out[i - 1] << " ";
    }
    std::cout << "\n";
    return 0;
}