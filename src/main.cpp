#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include "hashmap.h"
#include "minheap.h"
#include "timeutil.h"
#include "ringdeque.h"
#include "segtree.h"

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

bool isLeapYear(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

int daysInMonth(int y, int m) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m == 2 && isLeapYear(y)) return 29;
    return days[m - 1];
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
    // Asli calendar: mahina 1-12, din us mahine ke andar (leap year ke saath)
    int year  = std::stoi(d.substr(0, 4));
    int month = std::stoi(d.substr(5, 2));
    int day   = std::stoi(d.substr(8, 2));
    if (month < 1 || month > 12) return false;
    if (day < 1 || day > daysInMonth(year, month)) return false;
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
    // Asli ghadi: ghanta 00-23, minute 00-59, second 00-59
    int hour   = std::stoi(t.substr(0, 2));
    int minute = std::stoi(t.substr(3, 2));
    int second = std::stoi(t.substr(6, 2));
    return hour <= 23 && minute <= 59 && second <= 59;
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

// true agar a, b se "kharab" hai: heap mein kharab item upar rakhna hai
bool isWorse(const Item& a, const Item& b) {
    return isBetter(b, a);
}

// Apne heap (size K) se top K: upar hamesha sabse "kharab" item rehta hai.
// Heap bhar jane ke baad naya item tabhi andar aata hai jab wo upar wale
// (sabse kharab) se behtar ho; warna ek hi tulna mein reject.
Items topK(const Items& items, size_t k) {
    ItemHeap heap(isWorse);
    for (size_t i = 0; i < items.size(); i++) {
        if (heap.size() < k) {
            heap.push(items[i]);
        } else if (isBetter(items[i], heap.top())) {
            heap.pop();
            heap.push(items[i]);
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

struct Alert {
    std::string start;      // alert kab shuru hua
    std::string end;        // kab khatam hua
    std::string peakAt;     // peak kis waqt aaya
    int peak;               // window mein sabse zyada errors
    bool ongoing;           // file khatam hone tak chal raha tha
    long long startTs;      // shuru ka time (seconds mein)
    long long endTs;        // khatam hone ka time (seconds mein)
    int merged;             // kitne chhote alerts is incident mein jude hain
};

// Paas paas ke alerts ko ek incident mein jodo:
// agar agla alert pichhle ke khatam hone ke gap seconds ke andar shuru ho jaye.
std::vector<Alert> mergeAlerts(const std::vector<Alert>& in, long long gap) {
    std::vector<Alert> out;
    for (size_t i = 0; i < in.size(); i++) {
        const Alert& a = in[i];
        if (!out.empty() && a.startTs - out.back().endTs <= gap) {
            Alert& last = out.back();
            last.end = a.end;
            last.endTs = a.endTs;
            last.ongoing = a.ongoing;
            last.merged += a.merged;
            if (a.peak > last.peak) {
                last.peak = a.peak;
                last.peakAt = a.peakAt;
            }
        } else {
            out.push_back(a);
        }
    }
    return out;
}

// ---------- Week 5 Day 4: --range ke liye helpers ----------

// "2026-10-05 07:00" ko 1970 se minutes mein badlo. Format ya tareekh galat ho to false.
bool parseMinute(const std::string& text, long long& outMinute) {
    if (text.size() != 16) return false;
    if (text[4] != '-' || text[7] != '-' || text[10] != ' ' || text[13] != ':') return false;
    for (size_t i = 0; i < text.size(); i++) {
        if (i == 4 || i == 7 || i == 10 || i == 13) continue;
        if (text[i] < '0' || text[i] > '9') return false;
    }
    int year   = std::stoi(text.substr(0, 4));
    int month  = std::stoi(text.substr(5, 2));
    int day    = std::stoi(text.substr(8, 2));
    int hour   = std::stoi(text.substr(11, 2));
    int minute = std::stoi(text.substr(14, 2));
    if (month < 1 || month > 12) return false;
    if (day < 1 || day > daysInMonth(year, month)) return false;
    if (hour > 23 || minute > 59) return false;
    outMinute = toEpochSeconds(text.substr(0, 10), text.substr(11, 5) + ":00") / 60;
    return true;
}

// Ulta kaam: 1970 se minutes ko "YYYY-MM-DD HH:MM" mein badlo.
// (din ko tareekh mein badalne ka standard civil-from-days tareeka)
std::string minuteLabel(long long minute) {
    long long days = minute / 1440;
    long long inDay = minute % 1440;
    long long z = days + 719468;
    long long era = (z >= 0 ? z : z - 146096) / 146097;
    long long doe = z - era * 146097;
    long long yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    long long doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    long long mp = (5 * doy + 2) / 153;
    int d = static_cast<int>(doy - (153 * mp + 2) / 5 + 1);
    int m = static_cast<int>(mp < 10 ? mp + 3 : mp - 9);
    int y = static_cast<int>(yoe + era * 400 + (m <= 2 ? 1 : 0));
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d",
                  y, m, d, static_cast<int>(inDay / 60), static_cast<int>(inDay % 60));
    return std::string(buf);
}

// Range query: [fromMin, toMin) mein kitne ERROR aur sabse busy minute kaun sa.
// Har ERROR ka minute errorMinutes mein hai. Segment tree: har minute ek bucket.
void printRange(const std::vector<long long>& errorMinutes, long long fromMin, long long toMin,
                const std::string& fromText, const std::string& toText) {
    std::cout << "Range query [" << fromText << ", " << toText << ")  ("
              << (toMin - fromMin) << " minutes):\n";

    if (errorMinutes.empty()) {
        std::cout << "  Errors in range: 0\n";
        std::cout << "  Busiest minute:  none\n";
        return;
    }

    long long firstMin = *std::min_element(errorMinutes.begin(), errorMinutes.end());
    long long lastMin = *std::max_element(errorMinutes.begin(), errorMinutes.end());
    size_t buckets = static_cast<size_t>(lastMin - firstMin + 1);

    // Pehle aur aakhri error ke beech bahut lamba gap (galat tareekh wali line) ho to
    // minute-buckets ka tree bahut bada ban jata. Isliye hadd: 1,000,000 minute (~694 din).
    const size_t kMaxBuckets = 1000000;
    if (buckets > kMaxBuckets) {
        std::cout << "  Cannot answer: errors span " << buckets
                  << " minutes (limit " << kMaxBuckets
                  << "). Check the log for lines with a wrong date.\n";
        return;
    }

    std::vector<long long> counts(buckets, 0);
    for (size_t i = 0; i < errorMinutes.size(); i++) {
        counts[static_cast<size_t>(errorMinutes[i] - firstMin)]++;
    }
    SegTree tree(counts);

    // Maange hue range ko data ke andar tak kaato, phir [from, to) ko [l, r] banao
    long long lo = std::max(fromMin, firstMin);
    long long hi = std::min(toMin - 1, lastMin);
    if (lo > hi) {
        std::cout << "  Errors in range: 0\n";
        std::cout << "  Busiest minute:  none\n";
        return;
    }
    size_t l = static_cast<size_t>(lo - firstMin);
    size_t r = static_cast<size_t>(hi - firstMin);

    long long total = tree.sum(l, r);
    std::cout << "  Errors in range: " << total << "\n";
    if (tree.maxValue(l, r) == 0) {
        std::cout << "  Busiest minute:  none\n";
    } else {
        size_t at = tree.maxIndex(l, r);
        std::cout << "  Busiest minute:  " << minuteLabel(firstMin + static_cast<long long>(at))
                  << " (" << tree.maxValue(l, r) << " errors)\n";
    }
}

int main(int argc, char* argv[]) {
    std::string path = "data/sample.log";
    bool show = false;
    bool checkTop = false;
    int topN = 3;
    int spikeWindow = 0;
    int spikeThreshold = 0;
    int mergeGap = 0;
    bool useRange = false;
    std::string rangeFromText;
    std::string rangeToText;
    long long rangeFromMin = 0;
    long long rangeToMin = 0;
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
        } else if (arg == "--spike" && i + 2 < argc) {
            spikeWindow = std::atoi(argv[++i]);
            spikeThreshold = std::atoi(argv[++i]);
            if (spikeWindow <= 0 || spikeThreshold <= 0) {
                std::cerr << "--spike needs two positive numbers: window seconds and threshold\n";
                return 1;
            }
        } else if (arg == "--spike") {
            std::cerr << "--spike needs two positive numbers: window seconds and threshold\n";
            return 1;
        } else if (arg == "--merge" && i + 1 < argc) {
            mergeGap = std::atoi(argv[++i]);
            if (mergeGap <= 0) {
                std::cerr << "--merge needs a positive number of seconds\n";
                return 1;
            }
        } else if (arg == "--merge") {
            std::cerr << "--merge needs a positive number of seconds\n";
            return 1;
        } else if (arg == "--range" && i + 2 < argc) {
            rangeFromText = argv[++i];
            rangeToText = argv[++i];
            if (!parseMinute(rangeFromText, rangeFromMin) || !parseMinute(rangeToText, rangeToMin)) {
                std::cerr << "Bad time format (use YYYY-MM-DD HH:MM): "
                          << rangeFromText << " / " << rangeToText << "\n";
                return 1;
            }
            if (rangeFromMin >= rangeToMin) {
                std::cerr << "--range: first time must be earlier than the second\n";
                return 1;
            }
            useRange = true;
        } else if (arg == "--range") {
            std::cerr << "--range needs two times: \"YYYY-MM-DD HH:MM\" \"YYYY-MM-DD HH:MM\"\n";
            return 1;
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

    RingDeque window;     // pichhle W seconds ke ERROR events ka time
    std::vector<Alert> alerts;
    Alert current = Alert();
    bool inAlert = false;
    bool haveLast = false;
    long long lastTs = 0;
    std::string lastStamp;
    int outOfOrder = 0;
    std::vector<long long> errorMinutes;   // --range ke liye: har ERROR ka minute

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

        if (spikeWindow > 0) {
            long long ts = toEpochSeconds(e.date, e.time);
            std::string stamp = e.date + " " + e.time;
            if (haveLast && ts < lastTs) {
                outOfOrder++;                       // time peeche gaya: is line ko chhod do
            } else {
                haveLast = true;
                lastTs = ts;
                lastStamp = stamp;

                // 1) Window se bahar gaye purane events aage se nikaalo
                while (!window.empty() && window.front() <= ts - spikeWindow) {
                    window.pop_front();
                }
                // 2) ERROR ho to peeche se jodo
                if (e.level == "ERROR") {
                    window.push_back(ts);
                }
                // 3) Window mein kitne errors hain?
                int count = static_cast<int>(window.size());
                if (count >= spikeThreshold) {
                    if (!inAlert) {
                        inAlert = true;
                        current = Alert();
                        current.start = stamp;
                        current.startTs = ts;
                        current.merged = 1;
                        current.peak = count;
                        current.peakAt = stamp;
                    } else if (count > current.peak) {
                        current.peak = count;
                        current.peakAt = stamp;
                    }
                } else if (inAlert) {
                    current.end = stamp;
                    current.endTs = ts;
                    current.ongoing = false;
                    alerts.push_back(current);
                    inAlert = false;
                }
            }
        }

        codeCount.increment(e.code);

        if (e.level == "ERROR") {
            errors++;
            errorCodeCount.increment(e.code);
            errorsPerHour.increment(e.date + " " + e.time.substr(0, 2));
            if (useRange) {
                errorMinutes.push_back(toEpochSeconds(e.date, e.time) / 60);
            }
        } else if (e.level == "WARN") {
            warns++;
        } else {
            infos++;
        }
    }

    if (inAlert) {
        current.end = lastStamp;
        current.endTs = lastTs;
        current.ongoing = true;
        alerts.push_back(current);
    }
    if (mergeGap > 0) {
        alerts = mergeAlerts(alerts, mergeGap);
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

    if (spikeWindow > 0) {
        std::cout << "Spike alerts (window " << spikeWindow << " s, threshold "
                  << spikeThreshold << " errors";
        if (mergeGap > 0) {
            std::cout << ", merge gap " << mergeGap << " s";
        }
        std::cout << "):\n";
        for (size_t i = 0; i < alerts.size(); i++) {
            std::cout << "  ALERT " << (i + 1) << ": " << alerts[i].start
                      << " to " << alerts[i].end
                      << "  peak " << alerts[i].peak << " errors at "
                      << alerts[i].peakAt;
            if (alerts[i].merged > 1) {
                std::cout << "  [" << alerts[i].merged << " alerts merged]";
            }
            if (alerts[i].ongoing) {
                std::cout << "  (still active at end of file)";
            }
            std::cout << "\n";
        }
        std::cout << "Total spike alerts: " << alerts.size() << "\n";
        if (outOfOrder > 0) {
            std::cout << "Out-of-order lines skipped for spike detection: "
                      << outOfOrder << "\n";
        }
    }

    if (useRange) {
        printRange(errorMinutes, rangeFromMin, rangeToMin, rangeFromText, rangeToText);
    }

    if (!queryCode.empty()) {
        std::cout << "Query: code " << queryCode << " appeared "
                  << codeCount.get(queryCode) << " times\n";
    }
    return 0;
}