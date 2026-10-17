# Log Analyzer & Anomaly Finder

A C++ tool that reads server logs and answers questions fast, using data
structures chosen for the job: a hash map, a heap, a deque and a segment tree.

## Status
- Week 1: log generator, parser, input validation, level counts
- Week 2: custom hash map (chaining + rehash), error code counts,
  busiest hour, code lookup, benchmarks
- Week 3: custom heap, top K busiest hours, top-K benchmarks,
  36 automatic checks

## How to run
```
python tools/generate_logs.py
g++ -std=c++17 -O2 src/main.cpp -o loganalyzer
./loganalyzer data/sample.log
```

Options:
- `--show`  print every parsed entry
- `--code AUTH_FAILED`  how many times a code appeared
- `--top 5`  show the 5 busiest hours (default 3)
- `--check-top`  verify the heap result against a full sort

Large test file:
```
python tools/generate_logs.py 1000000 data/big.log
```

Run all checks (compiles everything and verifies results):
```
python tools/run_tests.py
```

## Log format
```
YYYY-MM-DD HH:MM:SS LEVEL CODE message
```
LEVEL is INFO, WARN or ERROR. Malformed lines are counted and skipped.

## Data structures
| Data structure | Used for | Status |
|---|---|---|
| Hash map (own implementation, chaining + rehash) | count errors per code and per hour | done |
| Heap (own implementation, size-K min-heap) | top K busiest hours | done |
| Deque | sliding window spike alerts | Week 4 |
| Segment tree | fast range queries | Week 5 |

## Benchmarks
Measured on a Windows laptop with g++ -O2. Numbers vary slightly between runs.

Parsing 1,000,000 lines (57 MB):
- Week 1 (parser and level counts): 0.95 s
- Week 2 (parser + three hash maps): 0.86 s

Counting errors per hour (median of 3 runs, 1,000,000 lines, 1,251 distinct hours):

| Method | Time |
|---|---|
| Vector with linear search | 3.47 s |
| std::unordered_map | 0.043 s |
| Own hash map | 0.049 s |

Own hash map was about 70x faster than the vector and about 13% slower
than std::unordered_map.

### Top K (Week 3)
Top 10 busiest time slots, 1,000,000 line file, median of 5 runs (milliseconds):

| Keys (n) | Full sort | std::partial_sort | std::priority_queue | Own heap | Own heap + early skip |
|---|---|---|---|---|---|
| 1,251 (hours) | 0.35 | 0.032 | 0.28 | 0.19 | 0.016 |
| 70,948 (minutes) | 35.0 | 3.2 | 21.7 | 15.7 | 0.51 |
| 200,641 (seconds) | 145 | 13.5 | 54.2 | 61.2 | 4.0 |

Own heap was about 2.2x faster than a full sort (minutes). Early skip made it
about 15x faster than the plain heap (seconds) and about 37x faster than a
full sort. The plain own heap and std::priority_queue were in the same range.
All methods return the same result. When K grows to about half of n, the heap
advantage disappears.