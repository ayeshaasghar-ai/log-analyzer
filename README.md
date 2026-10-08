# Log Analyzer & Anomaly Finder

A C++ tool that analyzes server logs using a hash map, heap, deque and segment tree.

## Status
Week 1 done: log generator, parser, level counts, input validation.

## How to run
1. python tools/generate_logs.py
2. g++ -std=c++17 -O2 src/main.cpp -o loganalyzer
3. ./loganalyzer data/sample.log

## Log format
YYYY-MM-DD HH:MM:SS LEVEL CODE message

## Baseline benchmark (Week 1)
Parsing 1,000,000 lines (57 MB): 0.95 seconds