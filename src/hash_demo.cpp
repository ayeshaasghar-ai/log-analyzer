#include <iostream>
#include <string>
#include <vector>

size_t hashString(const std::string& s) {
    size_t h = 5381;
    for (unsigned char c : s) {
        h = h * 33 + c;
    }
    return h;
}

int main() {
    const size_t buckets = 8;

    std::vector<std::string> keys = {
        "AUTH_FAILED", "DB_TIMEOUT", "DISK_FULL",
        "LOGIN_OK", "PAGE_LOAD", "HIGH_MEMORY"
    };

    std::vector<std::vector<std::string>> table(buckets);

    for (const auto& k : keys) {
        size_t h = hashString(k);
        size_t b = h % buckets;
        std::cout << k << " -> hash " << h << " -> bucket " << b << "\n";
        table[b].push_back(k);
    }

    std::cout << "\n";
    for (size_t i = 0; i < buckets; i++) {
        std::cout << "Bucket " << i << ": ";
        for (const auto& k : table[i]) {
            std::cout << k << " ";
        }
        std::cout << "\n";
    }
    return 0;
}