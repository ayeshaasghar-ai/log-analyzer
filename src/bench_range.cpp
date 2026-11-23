
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <iomanip>
#include <string>
#include <utility>

#include "timeutil.h"
#include "segtree.h"

using namespace std;

struct Query {
    size_t l;
    size_t r;
};

const int REPETITIONS = 5;
const int QUERY_COUNT = 1000;

// Brute force range sum: O(n)
vector<long long> bruteSums(
    const vector<long long>& values,
    const vector<Query>& queries
) {
    vector<long long> results;

    for (const auto& q : queries) {
        long long sum = 0;

        for (size_t i = q.l; i <= q.r; i++) {
            sum += values[i];
        }

        results.push_back(sum);
    }

    return results;
}

// Prefix sum range query: O(1)
vector<long long> prefixSums(
    const vector<long long>& prefix,
    const vector<Query>& queries
) {
    vector<long long> results;

    for (const auto& q : queries) {
        results.push_back(prefix[q.r + 1] - prefix[q.l]);
    }

    return results;
}

// Segment tree range sum: O(log n)
vector<long long> treeSums(
    const SegTree& tree,
    const vector<Query>& queries
) {
    vector<long long> results;

    for (const auto& q : queries) {
        results.push_back(
            tree.sum(
                static_cast<int>(q.l),
                static_cast<int>(q.r)
            )
        );
    }

    return results;
}

// Brute force range maximum: O(n)
vector<long long> bruteMax(
    const vector<long long>& values,
    const vector<Query>& queries
) {
    vector<long long> results;

    for (const auto& q : queries) {
        long long maximum = values[q.l];

        for (size_t i = q.l + 1; i <= q.r; i++) {
            maximum = max(maximum, values[i]);
        }

        results.push_back(maximum);
    }

    return results;
}

// Segment tree range maximum: O(log n)
vector<long long> treeMax(
    const SegTree& tree,
    const vector<Query>& queries
) {
    vector<long long> results;

    for (const auto& q : queries) {
        results.push_back(
            tree.maxValue(
                static_cast<int>(q.l),
                static_cast<int>(q.r)
            )
        );
    }

    return results;
}

// Measure execution time and return the median result.
template <typename Function>
pair<double, vector<long long>> benchmark(Function function) {
    vector<double> times;
    vector<long long> results;

    for (int i = 0; i < REPETITIONS; i++) {
        auto start = chrono::high_resolution_clock::now();

        results = function();

        auto end = chrono::high_resolution_clock::now();

        double milliseconds =
            chrono::duration<double, milli>(end - start).count();

        times.push_back(milliseconds);
    }

    sort(times.begin(), times.end());

    return {times[times.size() / 2], results};
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: bench_range <log_file>\n";
        return 1;
    }

    ifstream file(argv[1]);

    if (!file) {
        cerr << "Error: Could not open log file: "
             << argv[1] << "\n";
        return 1;
    }

    // Read ERROR timestamps from the log file.
    vector<long long> errorMinutes;

    string line;

    while (getline(file, line)) {
        istringstream iss(line);

        string date;
        string time;
        string level;

        if (!(iss >> date >> time >> level)) {
            continue;
        }

        if (level != "ERROR") {
            continue;
        }

        long long epochSeconds = toEpochSeconds(date, time);

        errorMinutes.push_back(epochSeconds / 60);
    }

    file.close();

    if (errorMinutes.empty()) {
        cerr << "Error: No ERROR entries found in log file.\n";
        return 1;
    }

    // Find the first and last minute.
    auto minMax = minmax_element(
        errorMinutes.begin(),
        errorMinutes.end()
    );

    long long firstMinute = *minMax.first;
    long long lastMinute = *minMax.second;

    // Create one bucket for every minute.
    size_t count = static_cast<size_t>(
        lastMinute - firstMinute + 1
    );

    vector<long long> counts(count, 0);

    for (long long minute : errorMinutes) {
        size_t index = static_cast<size_t>(
            minute - firstMinute
        );

        counts[index]++;
    }

    cout << "====================================\n";
    cout << "       RANGE QUERY BENCHMARK\n";
    cout << "====================================\n";

    cout << "ERROR entries: " << errorMinutes.size() << "\n";
    cout << "Minute buckets: " << counts.size() << "\n";
    cout << "Queries: " << QUERY_COUNT << "\n";
    cout << "Repetitions: " << REPETITIONS << "\n\n";

    // Build prefix sums.
    vector<long long> prefix(counts.size() + 1, 0);

    for (size_t i = 0; i < counts.size(); i++) {
        prefix[i + 1] = prefix[i] + counts[i];
    }

    // Build segment tree.
    SegTree tree(counts);

    // Generate deterministic random queries.
    vector<Query> queries;

    mt19937 rng(20261123);

    uniform_int_distribution<size_t> startDistribution(
        0,
        counts.size() - 1
    );

    for (int i = 0; i < QUERY_COUNT; i++) {
        size_t l = startDistribution(rng);

        size_t maxR = min(
            counts.size() - 1,
            l + 360
        );

        uniform_int_distribution<size_t> endDistribution(
            l,
            maxR
        );

        size_t r = endDistribution(rng);

        queries.push_back({l, r});
    }

    cout << "Running benchmarks...\n\n";

    // Benchmark range sum methods.
    auto bruteSumResult = benchmark([&]() {
        return bruteSums(counts, queries);
    });

    auto prefixSumResult = benchmark([&]() {
        return prefixSums(prefix, queries);
    });

    auto treeSumResult = benchmark([&]() {
        return treeSums(tree, queries);
    });

    // Benchmark range maximum methods.
    auto bruteMaxResult = benchmark([&]() {
        return bruteMax(counts, queries);
    });

    auto treeMaxResult = benchmark([&]() {
        return treeMax(tree, queries);
    });

    cout << fixed << setprecision(4);

    cout << "----------- RANGE SUM -----------\n";

    cout << "Brute Force: "
         << bruteSumResult.first << " ms\n";

    cout << "Prefix Sum:  "
         << prefixSumResult.first << " ms\n";

    cout << "Segment Tree: "
         << treeSumResult.first << " ms\n\n";

    cout << "---------- RANGE MAXIMUM --------\n";

    cout << "Brute Force: "
         << bruteMaxResult.first << " ms\n";

    cout << "Segment Tree: "
         << treeMaxResult.first << " ms\n\n";

    // Verify that all methods produce identical answers.
    bool sumCorrect =
        bruteSumResult.second == prefixSumResult.second &&
        bruteSumResult.second == treeSumResult.second;

    bool maxCorrect =
        bruteMaxResult.second == treeMaxResult.second;

    cout << "----------- VERIFICATION --------\n";

    cout << "Range Sum methods agree: "
         << (sumCorrect ? "YES" : "NO") << "\n";

    cout << "Range Max methods agree: "
         << (maxCorrect ? "YES" : "NO") << "\n";

    if (!sumCorrect || !maxCorrect) {
        cerr << "\nBENCHMARK FAILED: Results do not match.\n";
        return 1;
    }

    cout << "\nBENCHMARK PASSED\n";

    return 0;
}
