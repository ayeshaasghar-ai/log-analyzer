#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>
#include <chrono>
#include <cstdlib>
#include "hashmap.h"
#include "minheap.h"

typedef std::chrono::steady_clock Clock;
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

// Sab methods `work` (ek taza copy) par kaam karte hain.
// Copy banane ka time naapa nahi jata, taaki muqabla barabar ka ho.

// 1) Poori list sort karo, phir pehle K lo
Items topKBySort(Items& work, size_t k) {
    std::sort(work.begin(), work.end(), isBetter);
    if (work.size() > k) {
        work.resize(k);
    }
    return work;
}

// 2) std::partial_sort: sirf pehle K ko sort karta hai
Items topKByPartialSort(Items& work, size_t k) {
    if (work.size() > k) {
        std::partial_sort(work.begin(), work.begin() + k, work.end(), isBetter);
        work.resize(k);
    } else {
        std::sort(work.begin(), work.end(), isBetter);
    }
    return work;
}

// 3) std::priority_queue (size K): har item push, K se zyada ho to pop
Items topKByStdHeap(Items& work, size_t k) {
    std::priority_queue<Item, Items, bool (*)(const Item&, const Item&)> heap(isBetter);
    for (size_t i = 0; i < work.size(); i++) {
        heap.push(work[i]);
        if (heap.size() > k) {
            heap.pop();
        }
    }
    Items result;
    while (!heap.empty()) {
        result.push_back(heap.top());
        heap.pop();
    }
    std::reverse(result.begin(), result.end());
    return result;
}

// 4) Apna ItemHeap (size K): har item push, K se zyada ho to pop
Items topKByOwnHeap(Items& work, size_t k) {
    ItemHeap heap(isWorse);
    for (size_t i = 0; i < work.size(); i++) {
        heap.push(work[i]);
        if (heap.size() > k) {
            heap.pop();
        }
    }
    Items result;
    while (!heap.empty()) {
        result.push_back(heap.top());
        heap.pop();
    }
    std::reverse(result.begin(), result.end());
    return result;
}

// 5) Apna ItemHeap + "pehle top se compare":
//    heap bhar jane ke baad, jo item sabse kharab (top) se behtar nahi,
//    use heap mein daalte hi nahi (zyadatar items ek hi tulna mein reject)
Items topKByOwnHeapSkip(Items& work, size_t k) {
    ItemHeap heap(isWorse);
    for (size_t i = 0; i < work.size(); i++) {
        if (heap.size() < k) {
            heap.push(work[i]);
        } else if (isBetter(work[i], heap.top())) {
            heap.pop();
            heap.push(work[i]);
        }
    }
    Items result;
    while (!heap.empty()) {
        result.push_back(heap.top());
        heap.pop();
    }
    std::reverse(result.begin(), result.end());
    return result;
}

typedef Items (*Method)(Items&, size_t);

// Ek method ko `repeats` baar chalao, median time (milliseconds) lauto
double medianMs(Method m, const Items& items, size_t k, int repeats, Items& lastResult) {
    std::vector<double> times;
    for (int r = 0; r < repeats; r++) {
        Items work = items;                      // taza copy (timing ke bahar)
        Clock::time_point t0 = Clock::now();
        lastResult = m(work, k);
        Clock::time_point t1 = Clock::now();
        times.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());
    }
    std::sort(times.begin(), times.end());
    return times[times.size() / 2];
}

int main(int argc, char* argv[]) {
    std::string path = "data/big.log";
    std::string level = "minute";
    size_t k = 10;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--level" && i + 1 < argc) {
            level = argv[++i];
        } else if (arg == "--k" && i + 1 < argc) {
            int v = std::atoi(argv[++i]);
            if (v <= 0) {
                std::cerr << "--k needs a positive number\n";
                return 1;
            }
            k = static_cast<size_t>(v);
        } else if (arg.rfind("--", 0) == 0) {
            std::cerr << "Unknown option: " << arg << "\n";
            return 1;
        } else {
            path = arg;
        }
    }

    size_t timeChars = 0;
    if (level == "hour") timeChars = 2;
    else if (level == "minute") timeChars = 5;
    else if (level == "second") timeChars = 8;
    else {
        std::cerr << "--level must be hour, minute or second\n";
        return 1;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "File nahi khuli: " << path << "\n";
        return 1;
    }

    // ERROR lines ko key (date + time ka hissa) ke hisaab se ginte hain.
    // Yeh hissa timing mein shamil nahi hai.
    StringIntMap counts;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string d, t, lvl;
        if (!(ss >> d >> t >> lvl)) continue;
        if (lvl != "ERROR" || t.size() < timeChars) continue;
        counts.increment(d + " " + t.substr(0, timeChars));
    }
    Items items = counts.items();

    std::cout << "File: " << path << "\n";
    std::cout << "Level: " << level << "   distinct keys (n): " << items.size()
              << "   K: " << k << "\n\n";

    const int repeats = 5;
    Items rSort, rPartial, rStd, rOwn, rSkip;
    double tSort    = medianMs(topKBySort, items, k, repeats, rSort);
    double tPartial = medianMs(topKByPartialSort, items, k, repeats, rPartial);
    double tStd     = medianMs(topKByStdHeap, items, k, repeats, rStd);
    double tOwn     = medianMs(topKByOwnHeap, items, k, repeats, rOwn);
    double tSkip    = medianMs(topKByOwnHeapSkip, items, k, repeats, rSkip);

    std::cout << "Median of " << repeats << " runs (milliseconds):\n";
    std::cout << "  Full sort:              " << tSort    << " ms\n";
    std::cout << "  std::partial_sort:      " << tPartial << " ms\n";
    std::cout << "  std::priority_queue:    " << tStd     << " ms\n";
    std::cout << "  Own heap (ItemHeap):    " << tOwn     << " ms\n";
    std::cout << "  Own heap + early skip:  " << tSkip    << " ms\n";

    bool same = (rSort == rPartial) && (rSort == rStd) && (rSort == rOwn)
                && (rSort == rSkip);
    std::cout << "\nAll methods agree: " << (same ? "YES" : "NO") << "\n";
    return same ? 0 : 1;
}