# Log Analyzer & Anomaly Finder

A C++ tool that reads server logs and answers questions fast, using data
structures chosen for the job: a hash map, a heap, a deque and a segment tree.

## Status
- Week 1: log generator, parser, input validation, level counts
- Week 2: custom hash map (chaining + rehash), error code counts,
  busiest hour, code lookup, benchmarks

## How to run
```
python tools/generate_logs.py
g++ -std=c++17 -O2 src/main.cpp -o loganalyzer
./loganalyzer data/sample.log
```

Options:
- `--show`  print every parsed entry
- `--code AUTH_FAILED`  how many times a code appeared

Large test file:
```
python tools/generate_logs.py 1000000 data/big.log
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
| Heap | top K most frequent errors | Week 3 |
| Deque | sliding window spike alerts | Week 4 |
| Segment tree | fast range queries | Week 5 |

## Benchmarks
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
Scaling: when the input grew 10x (100K to 1M lines), the hash map time grew
about 9x, while the vector time grew about 60x.