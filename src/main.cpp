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

bool parseLine(const std::string& line, LogEntry& e) {
    std::istringstream ss(line);

    if (!(ss >> e.date >> e.time >> e.level >> e.code)) {
        return false;
    }

    std::getline(ss, e.message);
    if (!e.message.empty() && e.message[0] == ' ') {
        e.message.erase(0, 1);
    }
    return true;
}

int main() {
    std::ifstream file("data/sample.log");

    if (!file.is_open()) {
        std::cerr << "File nahi khuli!\n";
        return 1;
    }

    std::string line;
    int parsed = 0;
    int bad = 0;

    while (std::getline(file, line)) {
        LogEntry e;
        if (!parseLine(line, e)) {
            bad++;
            continue;
        }
        parsed++;

        if (parsed <= 3) {
            std::cout << "Date:    " << e.date    << "\n";
            std::cout << "Time:    " << e.time    << "\n";
            std::cout << "Level:   " << e.level   << "\n";
            std::cout << "Code:    " << e.code    << "\n";
            std::cout << "Message: " << e.message << "\n";
            std::cout << "---\n";
        }
    }

    std::cout << "Parsed: " << parsed << "  Bad lines: " << bad << "\n";
    return 0;
}