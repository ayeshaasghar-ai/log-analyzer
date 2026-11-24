# Log Analyzer & Anomaly Finder

A C++17 tool that reads server logs, analyzes errors, detects unusual activity, and answers range queries efficiently using custom data structures.

The project implements a hash map, min-heap, ring-buffer deque, and segment tree from scratch. Results are verified using automated tests, standard-library comparisons, and an independent Python reference.

## Project Status

- **Week 1:** Log generator, parser, input validation, and log-level counts.
- **Week 2:** Custom hash map with chaining and rehashing, error-code counts, busiest-hour analysis, code lookup, and benchmarks.
- **Week 3:** Custom min-heap, Top K busiest time slots, and benchmarks.
- **Week 4:** Time conversion, custom ring-buffer deque, sliding-window spike alerts, incident merging, and benchmarks.
- **Week 5:** Segment tree, range sum and maximum queries, CLI `--range` option, and benchmarking.
- **Automated testing:** 81 checks passing.

## Features

- Parse server logs and count INFO, WARN, and ERROR entries.
- Count errors by error code.
- Find the busiest hours and time slots.
- Search for a specific error code.
- Detect bursts of errors using a sliding time window.
- Merge nearby spike alerts into incidents.
- Query total errors and the busiest minute within a time range.
- Benchmark different algorithms and data structures.

## Requirements

- C++17-compatible compiler (such as g++)
- Python 3
- Windows PowerShell or a terminal

## How to Build and Run

Open PowerShell in the project directory:

```powershell
cd "D:\Projects\log-analyzer"
```

Generate the sample log:

```powershell
python tools/generate_logs.py
```

Compile the main program:

```powershell
g++ -std=c++17 -O2 src/main.cpp -o loganalyzer.exe
```

Run the analyzer:

```powershell
.\loganalyzer.exe data/sample.log
```

### Command-Line Options

| Option | Description |
|---|---|
| `--show` | Print every parsed entry |
| `--code AUTH_FAILED` | Count how many times a specific error code appeared |
| `--top 5` | Display the 5 busiest time slots (default: 3) |
| `--check-top` | Verify the heap result against a full sort |
| `--spike 300 40` | Alert when at least 40 errors occur within 300 seconds |
| `--merge 600` | Merge spike alerts that are at most 600 seconds apart (use with `--spike`) |
| `--range "2026-10-05 07:00" "2026-10-05 08:00"` | Query errors within a time range |

### Range Query Example

```powershell
.\loganalyzer.exe data/sample.log --range "2026-10-05 07:00" "2026-10-05 08:00"
```

The range uses minute-level timestamps and follows the **[from, to)** convention:

- The starting time is included.
- The ending time is excluded.
- The first time must be earlier than the second.
- Invalid date or time formats are rejected.

The range query reports the total number of errors and the busiest minute with its error count. If there are no errors in the selected period, the result reports zero errors and no busiest minute. If multiple minutes have the same maximum count, the earlier minute wins.

### Generate a Large Test File

Generate a log containing 1,000,000 lines:

```powershell
python tools/generate_logs.py 1000000 data/big.log
```

Run the analyzer on it:

```powershell
.\loganalyzer.exe data/big.log
```

## Automated Tests

Run the complete test suite:

```powershell
python tools/run_tests.py
```

The test runner compiles the project programs, executes functional and edge-case checks, verifies range-query behavior, and checks benchmark results.

**Latest verified result: 81/81 checks passed.**

## Log Format

Each log entry follows this format:

```text
YYYY-MM-DD HH:MM:SS LEVEL CODE message
```

Supported levels:

- `INFO`
- `WARN`
- `ERROR`

Malformed lines are counted and skipped.

## Spike Alerts

The sliding-window algorithm tracks ERROR timestamps using a deque.

As the window moves forward:

1. Expired events are removed from the front.
2. New ERROR events are added at the back.
3. The deque size gives the number of errors inside the current window.
4. An alert starts when the count reaches the threshold.
5. The alert ends when the count falls below the threshold.

Each alert reports its start time, end time, and peak error count.

Nearby alerts can be merged into a single incident using the `--merge` option.

### Example Output

For a hand-made log file with a 10-second window, threshold of 3 errors, and merge gap of 100 seconds:

```text
Spike alerts (window 10 s, threshold 3 errors, merge gap 100 s):
  ALERT 1: 2026-10-05 10:00:04 to 2026-10-05 10:02:02  peak 4 errors at 2026-10-05 10:00:06  [2 alerts merged]  (still active at end of file)
Total spike alerts: 1
```

The generated log contains an injected burst of authentication failures that the analyzer detects as an alert, including in the 1,000,000-line test file.

Timestamps are converted to seconds since 1970, allowing time-window calculations across days and leap years. Date conversion was checked day by day from 1970 to 2100.

## Data Structures

| Data Structure | Purpose | Implementation |
|---|---|---|
| Hash Map | Count errors by code and time slot | Custom chaining with rehashing |
| Min-Heap | Find Top K busiest time slots | Custom size-K min-heap |
| Deque | Track errors in a sliding window | Custom ring-buffer deque |
| Segment Tree | Answer range sum and maximum queries | Custom segment tree |

### Segment Tree

The segment tree supports:

- `sum(l, r)`: Calculate the sum over an inclusive index range.
- `maxValue(l, r)`: Find the maximum value over an inclusive index range.
- `set(i, value)`: Update an element.
- `add(i, delta)`: Add a value to an element.
- `size()`: Return the number of elements.

**Complexities:**

| Operation | Time Complexity |
|---|---|
| Build | O(n) |
| Range sum query | O(log n) |
| Range maximum query | O(log n) |
| Point update | O(log n) |

The command-line range query uses minute buckets. Its time interval is `[from, to)`, which is translated internally to the segment tree's inclusive index range.

## Benchmarks

Measured on a Windows laptop using g++ with `-O2`. Results may vary between runs and hardware.

### Log Parsing

Parsing 1,000,000 lines (57 MB):

| Version | Time |
|---|---:|
| Week 1: Parser and level counts | 0.95 s |
| Week 2: Parser + three hash maps | 0.86 s |

### Hash Map Benchmark

Counting errors per hour, median of 3 runs, 1,000,000 lines, and 1,251 distinct hours:

| Method | Time |
|---|---:|
| Vector with linear search | 3.47 s |
| `std::unordered_map` | 0.043 s |
| Own hash map | 0.049 s |

The custom hash map was about 70x faster than the vector and about 13% slower than `std::unordered_map`.

### Top K Benchmark (Week 3)

Top 10 busiest time slots, 1,000,000-line file, median of 5 runs (milliseconds):

| Keys (n) | Full sort | `std::partial_sort` | `std::priority_queue` | Own heap | Own heap + early skip |
|---|---:|---:|---:|---:|---:|
| 1,251 (hours) | 0.35 | 0.032 | 0.28 | 0.19 | 0.016 |
| 70,948 (minutes) | 35.0 | 3.2 | 21.7 | 15.7 | 0.51 |
| 200,641 (seconds) | 145 | 13.5 | 54.2 | 61.2 | 4.0 |

The custom heap was about 2.2x faster than a full sort for minutes. Early skip made it about 15x faster than the plain heap for seconds and about 37x faster than a full sort.

The plain custom heap and `std::priority_queue` were in the same range. All methods return the same result. When K grows to about half of n, the heap advantage disappears.

### Sliding Window Benchmark (Week 4)

Counting errors in a moving window over 1,000,000 lines, median of 3 runs (milliseconds):

| Window (seconds) | Naive recount | `std::deque` | Own RingDeque |
|---|---:|---:|---:|
| 300 | 16.4 | 9.4 | 8.7 |
| 3,600 | 80.6 | 9.0 | 8.6 |
| 14,400 | 295 | 8.8 | 8.8 |

The naive method recounts the window for every line, so its time grows with the window. It became about 18x slower when the window grew 48 times.

The deque performs constant work per event, so its time stayed nearly flat. At a window of 14,400 seconds, the custom deque was about 34x faster than naive recounting. The custom ring-buffer deque and `std::deque` were in the same range. All methods return the same result.

### Range Query Benchmark (Week 5, Day 5)

Benchmark configuration:

- Input file: `data/big.log`
- ERROR entries: 200,641
- Minute buckets: 75,004
- Queries: 1,000
- Repetitions: 5

#### Range Sum

| Method | Average Query Time |
|---|---:|
| Brute Force | 0.1683 ms |
| Prefix Sum | 0.0190 ms |
| Segment Tree | 0.3423 ms |

In this measured run, prefix sums were about 8.9x faster than brute force for range-sum queries.

#### Range Maximum

| Method | Average Query Time |
|---|---:|
| Brute Force | 0.1916 ms |
| Segment Tree | 0.4841 ms |

All benchmark methods agreed on their results, and the benchmark passed.

**Benchmark note:** These measurements cover query execution after the data structures have been built. Structure construction time is not included. Results are specific to this dataset and machine, not universal performance guarantees.

## Project Structure

```text
log-analyzer/
│
├── data/
│   ├── sample.log
│   └── big.log
│
├── src/
│   ├── main.cpp
│   ├── hashmap.h
│   ├── minheap.h
│   ├── ringdeque.h
│   ├── segtree.h
│   ├── timeutil.h
│   ├── bench_maps.cpp
│   ├── bench_topk.cpp
│   ├── bench_window.cpp
│   ├── bench_range.cpp
│   └── test_*.cpp
│
├── tools/
│   ├── generate_logs.py
│   └── run_tests.py
│
├── README.md
└── .gitignore
```

## Learning Outcomes

Through this project, I practiced:

- Implementing data structures from scratch in C++.
- Working with file handling, parsing, and input validation.
- Applying hashing, heaps, deques, and segment trees to practical problems.
- Understanding algorithmic time complexity.
- Comparing algorithms through repeatable benchmarks.
- Writing automated tests and checking edge cases.
- Using Git and GitHub to manage project development.