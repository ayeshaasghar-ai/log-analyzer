#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include "hashmap.h"

typedef std::chrono::steady_clock Clock;

double secondsBetween(Clock::time_point a, Clock::time_point b) {
    return std::chrono::duration<double>(b - a).count();
}

int main(int argc, char* argv[]) {
    std::string path = (argc > 1) ? argv[1] : "data/big.log";

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "File nahi khuli: " << path << "\n";
        return 1;
    }

    // Pehle saari keys (date + ghanta) memory mein le lo,
    // taaki sirf ginti ka time naape, file padhne ka nahi.
    std::vector<std::string> keys;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string d, tstr;
        if (!(ss >> d >> tstr)) continue;
        if (tstr.size() < 2) continue;
        keys.push_back(d + " " + tstr.substr(0, 2));
    }
    std::cout << "File: " << path << "\n";
    std::cout << "Keys loaded: " << keys.size() << "\n\n";

    // 1) Vector + linear search
    size_t distinctVec = 0;
    {
        Clock::time_point t0 = Clock::now();
        std::vector<std::pair<std::string, int>> v;
        for (size_t i = 0; i < keys.size(); i++) {
            bool found = false;
            for (size_t j = 0; j < v.size(); j++) {
                if (v[j].first == keys[i]) {
                    v[j].second++;
                    found = true;
                    break;
                }
            }
            if (!found) v.push_back(std::make_pair(keys[i], 1));
        }
        Clock::time_point t1 = Clock::now();
        distinctVec = v.size();
        std::cout << "Vector (linear search): " << secondsBetween(t0, t1)
                  << " s   distinct=" << distinctVec << "\n";
    }

    // 2) std::unordered_map
    size_t distinctStd = 0;
    {
        Clock::time_point t0 = Clock::now();
        std::unordered_map<std::string, int> m;
        for (size_t i = 0; i < keys.size(); i++) {
            m[keys[i]]++;
        }
        Clock::time_point t1 = Clock::now();
        distinctStd = m.size();
        std::cout << "std::unordered_map:     " << secondsBetween(t0, t1)
                  << " s   distinct=" << distinctStd << "\n";
    }

    // 3) Apna StringIntMap
    size_t distinctMine = 0;
    {
        Clock::time_point t0 = Clock::now();
        StringIntMap m;
        for (size_t i = 0; i < keys.size(); i++) {
            m.increment(keys[i]);
        }
        Clock::time_point t1 = Clock::now();
        distinctMine = m.size();
        std::cout << "My StringIntMap:        " << secondsBetween(t0, t1)
                  << " s   distinct=" << distinctMine << "\n";
    }

    bool same = (distinctVec == distinctStd) && (distinctStd == distinctMine);
    std::cout << "\nAll three agree: " << (same ? "YES" : "NO") << "\n";
    return same ? 0 : 1;
}