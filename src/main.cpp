#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <vector>
#include <algorithm>
#include <queue>
#include <cstdlib>
#include "hashmap.h"

struct LogEntry {
    std::string date;
    std::string time;
    std::string level;
    std::string code;
    std::string message;
};

std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
        start++;
    }
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        end--;
    }
    return s.substr(start, end - start);
}

bool isValidLevel(const std::string& level) {
    return level == "INFO" || level == "WARN" || level == "ERROR";
}

bool isValidDate(const std::string& d) {
    if (d.size() != 10) return false;
    for (size_t i = 0; i < d.size(); i++) {
        if (i == 4 || i == 7) {
            if (d[i] != '-') return false;
        } else if (!std::isdigit(static_cast<unsigned char>(d[i]))) {
            return false;
        }
    }
    return true;
}

bool isValidTime(const std::string& t) {
    if (t.size() != 8) return false;
    for (size_t i = 0; i < t.size(); i++) {
        if (i == 2 || i == 5) {
            if (t[i] != ':') return false;
        } else if (!std::isdigit(static_cast<unsigned char>(t[i]))) {
            return false;
        }
    }
    return true;
}

bool parseLine(const std::string& rawLine, LogEntry& e) {
    std::string line = trim(rawLine);
    std::istringstream ss(line);

    if (!(ss >> e.date >> e.time >> e.level >> e.code)) {
        return false;
    }
    if (!isValidDate(e.date) || !isValidTime(e.time)) {
        return false;
    }
    if (!isValidLevel(e.level)) {
        return false;
    }

    std::getline(ss, e.message);
    e.message = trim(e.message);
    return true;
}

typedef std::vector<std::pair<std::string, int>> Items;

void sortItems(Items& items) {
    std::sort(items.begin(), items.end(),
        [](const std::pair<std::string, int>& a, const std::pair<std::string, int>& b) {
            if (a.second != b.second) return a.second > b.second;
            return a.first < b.first;
        });
}

void printItems(const std::string& title, const Items& items, size_t limit) {
    std::cout << title << "\n";
    for (size_t i = 0; i < items.size() && i < limit; i++) {
        std::cout << "  " << items[i].first << ": " << items[i].second << "\n";
    }
}

typedef std::pair<std::string, int> Item;

// true agar a, b se "behtar" hai: zyada ginti, ya barabar ginti par chhota naam
bool isBetter(const Item& a, const Item& b) {
    if (a.second != b.second) return a.second > b.second;
    return a.first < b.first;
}

// Min-heap of size K: upar hamesha sabse "kharab" item rehta hai.
// Naya item aaye to push karo; K se zyada ho jaye to upar wala (sabse kharab) nikaal do.
Items topK(const Items& items, size_t k) {
    std::priority_queue<Item, std::vector<Item>,
                        bool (*)(const Item&, const Item&)> heap(isBetter);
    for (size_t i = 0; i < items.size(); i++) {
        heap.push(items[i]);
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

int main(int argc, char* argv[]) {
    std::string path = "data/sample.log";
    bool show = false;
    bool checkTop = false;
    int topN = 3;
    std::string queryCode;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--show") {
            show = true;
        } else if (arg == "--code" && i + 1 < argc) {
            queryCode = argv[++i];
        } else if (arg == "--top" && i + 1 < argc) {
            topN = std::atoi(argv[++i]);
            if (topN <= 0) {
                std::cerr << "--top needs a positive number\n";
                return 1;
            }
        } else if (arg == "--check-top") {
            checkTop = true;
        } else if (arg.rfind("--", 0) == 0) {
            std::cerr << "Unknown option: " << arg << "\n";
            return 1;
        } else {
            path = arg;
        }
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "File nahi khuli: " << path << "\n";
        return 1;
    }

    StringIntMap codeCount;
    StringIntMap errorCodeCount;
    StringIntMap errorsPerHour;

    std::string line;
    int parsed = 0;
    int bad = 0;
    int blank = 0;
    int infos = 0;
    int warns = 0;
    int errors = 0;

    while (std::getline(file, line)) {
        if (line.find_first_not_of(" \t\r") == std::string::npos) {
            blank++;
            continue;
        }

        LogEntry e;
        if (!parseLine(line, e)) {
            bad++;
            continue;
        }
        parsed++;

        if (show) {
            std::cout << e.level << " " << e.code
                      << " [" << e.message << "]\n";
        }

        codeCount.increment(e.code);

        if (e.level == "ERROR") {
            errors++;
            errorCodeCount.increment(e.code);
            errorsPerHour.increment(e.date + " " + e.time.substr(0, 2));
        } else if (e.level == "WARN") {
            warns++;
        } else {
            infos++;
        }
    }

    std::cout << "File:   " << path << "\n";
    std::cout << "Parsed: " << parsed << "  Bad lines: " << bad
              << "  Blank: " << blank << "\n";
    std::cout << "INFO:   " << infos  << "\n";
    std::cout << "WARN:   " << warns  << "\n";
    std::cout << "ERROR:  " << errors << "\n";

    Items allItems = codeCount.items();
    sortItems(allItems);
    printItems("All codes (most frequent first):", allItems, 10);

    Items errItems = errorCodeCount.items();
    sortItems(errItems);
    printItems("ERROR codes (most frequent first):", errItems, 10);

    if (!errItems.empty()) {
        std::cout << "Most frequent error: " << errItems[0].first
                  << " (" << errItems[0].second << " times)\n";
    }

    Items hourItems = errorsPerHour.items();
    Items busiest = topK(hourItems, static_cast<size_t>(topN));
    printItems("Busiest hours (ERROR count):", busiest, busiest.size());

    if (checkTop) {
        sortItems(hourItems);
        if (hourItems.size() > static_cast<size_t>(topN)) {
            hourItems.resize(static_cast<size_t>(topN));
        }
        std::cout << "Top-K heap matches full sort: "
                  << (hourItems == busiest ? "YES" : "NO") << "\n";
    }

    if (!queryCode.empty()) {
        std::cout << "Query: code " << queryCode << " appeared "
                  << codeCount.get(queryCode) << " times\n";
    }
    return 0;
}