#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include "timeutil.h"

// "2026-10-05 07:00" ko 1970 se minutes mein badlo. Format galat ho to false.
bool parseMinute(const std::string& text, long long& outMinute) {
    if (text.size() != 16) return false;
    if (text[4] != '-' || text[7] != '-' || text[10] != ' ' || text[13] != ':') return false;
    for (size_t i = 0; i < text.size(); i++) {
        if (i == 4 || i == 7 || i == 10 || i == 13) continue;
        if (text[i] < '0' || text[i] > '9') return false;
    }
    int month  = std::stoi(text.substr(5, 2));
    int day    = std::stoi(text.substr(8, 2));
    int hour   = std::stoi(text.substr(11, 2));
    int minute = std::stoi(text.substr(14, 2));
    if (month < 1 || month > 12 || day < 1 || day > 31) return false;
    if (hour > 23 || minute > 59) return false;
    outMinute = toEpochSeconds(text.substr(0, 10), text.substr(11, 5) + ":00") / 60;
    return true;
}

int main(int argc, char* argv[]) {
    std::string path = "data/sample.log";
    std::string fromText;
    std::string toText;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--from" && i + 1 < argc) {
            fromText = argv[++i];
        } else if (arg == "--to" && i + 1 < argc) {
            toText = argv[++i];
        } else if (arg.rfind("--", 0) == 0) {
            std::cerr << "Unknown option: " << arg << "\n";
            return 1;
        } else {
            path = arg;
        }
    }

    if (fromText.empty() || toText.empty()) {
        std::cerr << "Usage: range_prefix [file] --from \"YYYY-MM-DD HH:MM\" --to \"YYYY-MM-DD HH:MM\"\n";
        return 1;
    }

    long long fromMin = 0;
    long long toMin = 0;
    if (!parseMinute(fromText, fromMin) || !parseMinute(toText, toMin)) {
        std::cerr << "Bad time format (use YYYY-MM-DD HH:MM): " << fromText << " / " << toText << "\n";
        return 1;
    }
    if (fromMin >= toMin) {
        std::cerr << "--from must be earlier than --to\n";
        return 1;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "File nahi khuli: " << path << "\n";
        return 1;
    }

    // Har ERROR line ka minute (1970 se) ek list mein
    std::vector<long long> errorMinutes;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string d, t, level;
        if (!(ss >> d >> t >> level)) continue;
        if (d.size() != 10 || t.size() != 8) continue;
        if (level != "ERROR") continue;
        errorMinutes.push_back(toEpochSeconds(d, t) / 60);
    }

    std::cout << "File: " << path << "\n";
    std::cout << "Errors: " << errorMinutes.size() << "\n";

    // Tareeka 1: brute force, saari errors scan karo
    long long brute = 0;
    for (size_t i = 0; i < errorMinutes.size(); i++) {
        if (errorMinutes[i] >= fromMin && errorMinutes[i] < toMin) brute++;
    }

    // Tareeka 2: prefix sums, har minute ke liye ek bucket
    long long viaPrefix = 0;
    if (!errorMinutes.empty()) {
        long long firstMin = *std::min_element(errorMinutes.begin(), errorMinutes.end());
        long long lastMin = *std::max_element(errorMinutes.begin(), errorMinutes.end());
        size_t buckets = static_cast<size_t>(lastMin - firstMin + 1);

        std::vector<long long> counts(buckets, 0);
        for (size_t i = 0; i < errorMinutes.size(); i++) {
            counts[static_cast<size_t>(errorMinutes[i] - firstMin)]++;
        }
        std::vector<long long> prefix(buckets + 1, 0);
        for (size_t i = 0; i < buckets; i++) {
            prefix[i + 1] = prefix[i] + counts[i];
        }
        std::cout << "Minute buckets: " << buckets << " (pehle error se aakhri error tak)\n";

        // Range ko data ke andar tak kaato (half-open: [from, to))
        long long l = std::max(fromMin, firstMin);
        long long r = std::min(toMin, lastMin + 1);
        if (l < r) {
            viaPrefix = prefix[static_cast<size_t>(r - firstMin)]
                      - prefix[static_cast<size_t>(l - firstMin)];
        }
    }

    std::cout << "Range: [" << fromText << ", " << toText << ")  = "
              << (toMin - fromMin) << " minutes\n";
    std::cout << "Brute force (saari errors scan):  " << brute << "\n";
    std::cout << "Prefix sums (2 lookups):          " << viaPrefix << "\n";
    bool same = (brute == viaPrefix);
    std::cout << "Both agree: " << (same ? "YES" : "NO") << "\n";
    return same ? 0 : 1;
}