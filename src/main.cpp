#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

struct LogEntry {
    std::string date;
    std::string time;
    std::string level;
    std::string code;
    std::string message;
};

bool isValidLevel(const std::string& level) {
    return level == "INFO" || level == "WARN" || level == "ERROR";
}

bool parseLine(const std::string& line, LogEntry& e) {
    std::istringstream ss(line);

    if (!(ss >> e.date >> e.time >> e.level >> e.code)) {
        return false;
    }

    if (!isValidLevel(e.level)) {
        return false;
    }

    std::getline(ss, e.message);
    if (!e.message.empty() && e.message[0] == ' ') {
        e.message.erase(0, 1);
    }
    return true;
}

int main(int argc, char* argv[]) {
    std::string path = "data/sample.log";
    if (argc > 1) {
        path = argv[1];
    }

    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "File nahi khuli: " << path << "\n";
        return 1;
    }

    std::string line;
    int parsed = 0;
    int bad = 0;
    int infos = 0;
    int warns = 0;
    int errors = 0;

    while (std::getline(file, line)) {
        LogEntry e;
        if (!parseLine(line, e)) {
            bad++;
            continue;
        }
        parsed++;

        if (e.level == "ERROR") {
            errors++;
        } else if (e.level == "WARN") {
            warns++;
        } else {
            infos++;
        }
    }

    std::cout << "File:   " << path << "\n";
    std::cout << "Parsed: " << parsed << "  Bad lines: " << bad << "\n";
    std::cout << "INFO:   " << infos  << "\n";
    std::cout << "WARN:   " << warns  << "\n";
    std::cout << "ERROR:  " << errors << "\n";
    return 0;
}