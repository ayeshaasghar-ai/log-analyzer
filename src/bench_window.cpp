#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include "timeutil.h"
#include "ringdeque.h"

typedef std::chrono::steady_clock Clock;

struct Event {
    long long ts;      // time (seconds)
    bool isError;
};

struct Result {
    long long sumCounts;   // har line par window ki ginti ka jod
    long long fires;       // kitni lines par ginti threshold tak pahunchi
    bool operator==(const Result& o) const {
        return sumCounts == o.sumCounts && fires == o.fires;
    }
};

// 1) Naive: har line par window ke errors peeche jaa kar dobara ginte hain
Result naiveRecount(const std::vector<Event>& events, long long window, int threshold) {
    std::vector<long long> errs;
    Result r = {0, 0};
    for (size_t i = 0; i < events.size(); i++) {
        if (events[i].isError) {
            errs.push_back(events[i].ts);
        }
        long long limit = events[i].ts - window;
        long long count = 0;
        for (size_t j = errs.size(); j > 0 && errs[j - 1] > limit; j--) {
            count++;
        }
        r.sumCounts += count;
        if (count >= threshold) r.fires++;
    }
    return r;
}

// 2) std::deque
Result withStdDeque(const std::vector<Event>& events, long long window, int threshold) {
    std::deque<long long> w;
    Result r = {0, 0};
    for (size_t i = 0; i < events.size(); i++) {
        long long limit = events[i].ts - window;
        while (!w.empty() && w.front() <= limit) {
            w.pop_front();
        }
        if (events[i].isError) {
            w.push_back(events[i].ts);
        }
        long long count = static_cast<long long>(w.size());
        r.sumCounts += count;
        if (count >= threshold) r.fires++;
    }
    return r;
}

// 3) Apna RingDeque
Result withRingDeque(const std::vector<Event>& events, long long window, int threshold) {
    RingDeque w;
    Result r = {0, 0};
    for (size_t i = 0; i < events.size(); i++) {
        long long limit = events[i].ts - window;
        while (!w.empty() && w.front() <= limit) {
            w.pop_front();
        }
        if (events[i].isError) {
            w.push_back(events[i].ts);
        }
        long long count = static_cast<long long>(w.size());
        r.sumCounts += count;
        if (count >= threshold) r.fires++;
    }
    return r;
}

typedef Result (*Method)(const std::vector<Event>&, long long, int);

// Method ko `repeats` baar chalao, median time (milliseconds) lauto
double medianMs(Method m, const std::vector<Event>& events, long long window,
                int threshold, int repeats, Result& last) {
    std::vector<double> times;
    for (int r = 0; r < repeats; r++) {
        Clock::time_point t0 = Clock::now();
        last = m(events, window, threshold);
        Clock::time_point t1 = Clock::now();
        times.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());
    }
    std::sort(times.begin(), times.end());
    return times[times.size() / 2];
}

int main(int argc, char* argv[]) {
    std::string path = "data/big.log";
    int threshold = 40;
    std::vector<long long> windows = {300, 3600, 14400};

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--threshold" && i + 1 < argc) {
            threshold = std::atoi(argv[++i]);
            if (threshold <= 0) {
                std::cerr << "--threshold needs a positive number\n";
                return 1;
            }
        } else if (arg == "--windows" && i + 1 < argc) {
            // Ek hi window dena ho to: --windows 300
            long long w = std::atoll(argv[++i]);
            if (w <= 0) {
                std::cerr << "--windows needs a positive number of seconds\n";
                return 1;
            }
            windows.clear();
            windows.push_back(w);
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

    // Pehle saari lines ka time memory mein (yeh hissa timing mein shamil nahi)
    std::vector<Event> events;
    std::string line;
    long long lastTs = -1;
    size_t errors = 0;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string d, t, level;
        if (!(ss >> d >> t >> level)) continue;
        if (d.size() != 10 || t.size() != 8) continue;
        long long ts = toEpochSeconds(d, t);
        if (ts < lastTs) continue;                 // out-of-order chhod do
        lastTs = ts;
        Event e;
        e.ts = ts;
        e.isError = (level == "ERROR");
        if (e.isError) errors++;
        events.push_back(e);
    }

    std::cout << "File: " << path << "   lines: " << events.size()
              << "   errors: " << errors << "   threshold: " << threshold << "\n";

    const int repeats = 3;
    bool allAgree = true;
    for (size_t k = 0; k < windows.size(); k++) {
        Result rNaive, rStd, rRing;
        double tNaive = medianMs(naiveRecount, events, windows[k], threshold, repeats, rNaive);
        double tStd   = medianMs(withStdDeque, events, windows[k], threshold, repeats, rStd);
        double tRing  = medianMs(withRingDeque, events, windows[k], threshold, repeats, rRing);

        bool same = (rNaive == rStd) && (rStd == rRing);
        allAgree = allAgree && same;

        std::cout << "\nWindow " << windows[k] << " s (median of " << repeats
                  << " runs, milliseconds):\n";
        std::cout << "  Naive recount:   " << tNaive << " ms\n";
        std::cout << "  std::deque:      " << tStd   << " ms\n";
        std::cout << "  Own RingDeque:   " << tRing  << " ms\n";
        std::cout << "  Lines at or above threshold: " << rRing.fires << "\n";
    }

    std::cout << "\nAll methods agree: " << (allAgree ? "YES" : "NO") << "\n";
    return allAgree ? 0 : 1;
}