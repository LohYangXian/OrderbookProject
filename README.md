# Low-Latency Order Book Engine (C++)

This project is a research-style exploration of building a **low-latency order matching engine in C++**, mainly inspired from research such as [C++ Design Patterns for Low-Latency Applications including High-Frequency Trading](https://arxiv.org/pdf/2309.04259).

The goal is to implement a baseline order book engine, then iteratively apply optimizations and benchmark their impact on performance.  

All results and learnings will be documented here in detail.

---

## Project Vision

- Build a **production-style order book engine** that supports:
  - Limit Orders
  - Market Orders
  - Cancels
  - Modifications
  - Order Matching
- Replay **real exchange order flows** (Binance WebSocket) for realism.
- Optimize aggressively with a focus on **nanosecond-level latency** and **throughput scaling**.
- Track and publish **performance improvements** at each iteration.

This project is meant as my pet project to learn **C++**, **systems-level thinking**, and **low-latency design tradeoffs**.

---

## Project Layout
<!-- TODO: Add directory structure and file descriptions -->
- Work in progress...


---

## Tech Stack

- **Language**: C++20
- **Build**: Bazel
- **Testing/Benchmarking**:
  - Google Benchmark
  - `std::chrono` + `rdtsc`
  - `perf` (Linux) for cache misses/branch mispredicts
- **Data Source**:
  - Binance WebSocket API (`btcusdt@depth`, `btcusdt@trade`)
  - Historical feeds for reproducibility

---

## Benchmarking Methodology

At each iteration:
1. Replay the **same set of orders** through the engine.
2. Record metrics:
   - Average latency (ns/op)
   - p50, p99, p999 latency
   - Throughput (orders/sec)
   - Memory allocations, cache misses (optional)
3. Log results in `results.csv`
4. Plot latency/throughput across iterations

## Versions of Orderbook Engine
<!-- TODO: Add Versions along the way -->
 - Work in progress...


## References
- [C++ Design Patterns for Low-Latency Applications including High-Frequency Trading](https://arxiv.org/pdf/2309.04259)