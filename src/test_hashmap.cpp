#include <iostream>
#include <unordered_map>
#include <string>
#include <cstdlib>
#include "hashmap.h"

int main() {
    StringIntMap mine;
    std::unordered_map<std::string, int> ref;

    std::srand(42);
    for (int i = 0; i < 100000; i++) {
        std::string key = "KEY_" + std::to_string(std::rand() % 5000);
        mine.increment(key);
        ref[key]++;
    }

    bool ok = (mine.size() == ref.size());

    for (const auto& kv : ref) {
        if (mine.get(kv.first) != kv.second) {
            std::cout << "Mismatch: " << kv.first << "\n";
            ok = false;
            break;
        }
    }

    if (mine.get("NOT_THERE") != 0) {
        ok = false;
    }

    std::cout << "Keys: " << mine.size()
              << "  Buckets: " << mine.bucketCount() << "\n";
    std::cout << (ok ? "TEST PASSED" : "TEST FAILED") << "\n";
    return ok ? 0 : 1;
}