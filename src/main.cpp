#include <iostream>
#include <fstream>
#include <string>

int main() {
    std::ifstream file("data/sample.log");

    if (!file.is_open()) {
        std::cerr << "File nahi khuli!\n";
        return 1;
    }

    std::string line;
    int count = 0;

    while (std::getline(file, line)) {
        count++;
        if (count <= 3) {
            std::cout << line << "\n";
        }
    }
    

    std::cout << "Total lines: " << count << "\n";
    return 0;
}