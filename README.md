# Low-Latency Order Book Engine (C++)

This project is a research-style exploration of building a **low-latency order matching engine in C++**, mainly inspired from research such as [C++ Design Patterns for Low-Latency Applications including High-Frequency Trading](https://arxiv.org/pdf/2309.04259).

The goal is to implement a baseline order book engine, then iteratively apply optimizations and benchmark their impact on performance.  

All results and learnings will be documented here in detail.

---

## Project Vision

- Build a **order book engine** that supports:
  - Limit Orders
  - Order Matching
  - Trade Generation
- Test using **real exchange order flows** (Binance WebSocket) and snapshots for realism.
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

### v0: Baseline
- Build websocket client to ingest live order flow.
- Save raw messages to be used as snapshots for replay. (For Performance consistency)
- Implement basic order book engine with naive data structures (e.g. `std::map`, `std::list`).
- Support limit orders, order matching and trade generation.
- Cover workflow from order ingestion to matching to trade generation.
- Measure baseline latency and throughput using `std::chrono`, `rdtsc`, and `perf`.

## References

### C++ Resources
- [C++ Design Patterns for Low-Latency Applications including High-Frequency Trading](https://arxiv.org/pdf/2309.04259)

### Data Sources
- [Binance WebSocket API Documentation](https://developers.binance.com/docs/binance-spot-api-docs/web-socket-streams)
- [Binance How to manage a local order book correctly](https://developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/How-to-manage-a-local-order-book-correctly)
- [WebSocket API: Order Book](https://developers.binance.com/docs/derivatives/usds-margined-futures/market-data/websocket-api)

### Order Book Implementations
- Market Microstructure Theory, by Maureen O'Hara