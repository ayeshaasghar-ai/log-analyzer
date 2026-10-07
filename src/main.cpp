#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>

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

int main(int argc, char* argv[]) {
    std::string path = "data/sample.log";
    if (argc > 1) {
        path = argv[1];
    }
    bool show = (argc > 2);

    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "File nahi khuli: " << path << "\n";
        return 1;
    }

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

        if (e.level == "ERROR") {
            errors++;
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
    return 0;
}