#include <iostream>
#include <string>
#include <vector>

void printArray(const std::string& label, const std::vector<int>& v) {
    std::cout << label << " [";
    for (size_t i = 0; i < v.size(); i++) {
        if (i > 0) std::cout << " ";
        std::cout << v[i];
    }
    std::cout << "]\n";
}

// Tareeka 1 (brute force): l se r tak (dono shamil) ek ek item jodo
int bruteSum(const std::vector<int>& a, int l, int r, int& looked) {
    int sum = 0;
    looked = 0;
    for (int i = l; i <= r; i++) {
        sum += a[i];
        looked++;
    }
    return sum;
}

// Prefix array: prefix[i] = pehle i items ka jod
std::vector<int> buildPrefix(const std::vector<int>& a) {
    std::vector<int> prefix(a.size() + 1, 0);
    for (size_t i = 0; i < a.size(); i++) {
        prefix[i + 1] = prefix[i] + a[i];
    }
    return prefix;
}

int main() {
    // Har minute mein kitne errors aaye (minute 0 se 9 tak)
    std::vector<int> errors = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};

    std::cout << "=== Part 1: data ===\n";
    std::cout << "minute  [0 1 2 3 4 5 6 7 8 9]\n";
    printArray("errors ", errors);

    // ---------- Part 2: brute force ----------
    std::cout << "\n=== Part 2: brute force (har query par l se r tak jodo) ===\n";
    int queries[3][2] = {{2, 5}, {0, 9}, {7, 7}};
    for (int q = 0; q < 3; q++) {
        int l = queries[q][0];
        int r = queries[q][1];
        int looked = 0;
        int sum = bruteSum(errors, l, r, looked);
        std::cout << "sum(" << l << ", " << r << ") = " << sum
                  << "   (" << looked << " items dekhe)\n";
    }

    // ---------- Part 3: prefix sums ----------
    std::cout << "\n=== Part 3: prefix sums (ek baar banao, phir har query 2 lookups) ===\n";
    std::vector<int> prefix = buildPrefix(errors);
    printArray("prefix ", prefix);
    for (int q = 0; q < 3; q++) {
        int l = queries[q][0];
        int r = queries[q][1];
        int sum = prefix[r + 1] - prefix[l];
        std::cout << "sum(" << l << ", " << r << ") = prefix[" << (r + 1) << "] - prefix["
                  << l << "] = " << prefix[r + 1] << " - " << prefix[l] << " = " << sum
                  << "   (2 items dekhe)\n";
    }

    // ---------- Part 4: update ka masla ----------
    std::cout << "\n=== Part 4: ek value badli to? ===\n";
    errors[4] = 10;     // minute 4 ke errors 5 se 10 ho gaye (+5)
    std::vector<int> newPrefix = buildPrefix(errors);
    int changed = 0;
    for (size_t i = 0; i < prefix.size(); i++) {
        if (prefix[i] != newPrefix[i]) changed++;
    }
    printArray("errors ", errors);
    printArray("prefix ", newPrefix);
    std::cout << "prefix ki " << changed << " entries badalni padi (kul " << prefix.size()
              << " mein se)\n";
    int looked = 0;
    std::cout << "sum(2, 5) ab = " << (newPrefix[6] - newPrefix[2])
              << "   (brute force: " << bruteSum(errors, 2, 5, looked) << ")\n";

    // ---------- Part 5: max ka sawal ----------
    std::cout << "\n=== Part 5: sabse zyada errors wala minute (max) ===\n";
    int best = errors[2];
    for (int i = 3; i <= 5; i++) {
        if (errors[i] > best) best = errors[i];
    }
    std::cout << "max(2, 5) = " << best << "   (prefix sums se yeh nahi nikal sakta)\n";
    return 0;
}