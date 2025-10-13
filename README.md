# Low-Latency Order Book Engine (C++)

This project is a research-style exploration of building a **low-latency order matching engine in C++**, mainly inspired from research such as [C++ Design Patterns for Low-Latency Applications including High-Frequency Trading](https://arxiv.org/pdf/2309.04259).

The goal is to implement a baseline order book engine, then iteratively apply optimizations and benchmark their impact on performance.  

All results and learnings will be documented here in detail.

---

## Project Vision

- Build a **order book engine** that supports:
  - Limit Orders
  - Add / Modify / Cancel Orders
  - Order Matching
  - Trade Generation
- Support **generic JSON orders** for flexibility
- Client - Server architecture to simulate real-world order submission and acknowledgment
- Measure **Round Trip Time (RTT)** from order submission to acknowledgment
- Optimize aggressively with a focus on **nanosecond-level latency** and **throughput scaling**.
- Track and publish **performance improvements** at each iteration.

This project is meant as my pet project to learn **C++**, **systems-level thinking**, and **low-latency design tradeoffs**.

--- 

## Design

          +-----------------+
          |      Client     |
          | (JSON Orders)   |
          +--------+--------+
                   |
                   v
          +-----------------+
          |      Server     |
          | (Order Router)  |
          +--------+--------+
                   |
                   v
          +-----------------+
          | Order Book      |
          | Engine (C++)    |
          | - Add/Cancel    |
          | - Modify        |
          | - Match Orders  |
          | - Generate Trades|
          +--------+--------+
                   |
                   v
          +-----------------+
          |      Server     |
          | (JSON Response) |
          +--------+--------+
                   |
                   v
          +-----------------+
          |      Client     |
          +-----------------+


Client sends JSON orders to Server -> Server processes orders through Order Book Engine -> Server returns JSON responses indicating trade execution or order addition.

**Round Trip Time** (RTT) is measured from Client order submission to Server acknowledgment.

Performance metrics such as **latency** (ns/op), **throughput** (orders/sec), and **memory usage** are tracked at each iteration.

---

## Tech Stack

- **Language**: C++20
- **Build**: Bazel
- **Testing/Benchmarking**:
  - Google Benchmark
  - `std::chrono` + `rdtsc`
  - `Apple's Time Profiler` for CPU profiling
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


### Using `Apple's Time Profiler` to Measure Performance

The time profiler is attached to the server process to measure CPU usage and latency.

---

## How to run it

### Build the Server:
```bash
bazel build //src:main_server
```

### Run the Server:
```bash
bazel run //src:main_server
```

### Run the Tests:
```bash
bazel test //tests:orderbook_test
```

### Run the Client:
Just open `py_client/client.ipynb` in Jupyter and run the cells.

---

## Versions of Orderbook Engine

### v0: Baseline Implementations
- Implement basic order book engine with naive data structures (e.g. `std::map`, `std::list`).
- Support limit orders, order adding, order modifying, order cancelling, order matching and trade generation.
- Basic mutex locking for thread safety.
- Cover workflow from order ingestion to matching to trade generation.
- Measure baseline latency and throughput using `std::chrono`, `rdtsc`, and `Apple's Time Profiler`.

#### v0: Benchmark Results

**Single-threaded (1 thread, 50,000 orders):**

- **Throughput:** 2,661 orders/sec
- **RTT stats (ALL):** mean = 323.26μs, p95 = 435μs, p99 = 666μs
- **NEW orders:** mean = 329.82μs, p95 = 444μs, p99 = 677.67μs
- **MODIFY orders:** mean = 315.66μs, p95 = 409μs, p99 = 632μs
- **CANCEL orders:** mean = 292.80μs, p95 = 396μs, p99 = 616.90μs

![RTT Histogram 1 thread](/results/v0/RTT_ALL_single.png)

![RTT Histogram NEW](/results/v0/RTT_NEW_single.png)
![RTT Histogram MODIFY](/results/v0/RTT_MODIFY_single.png)
![RTT Histogram CANCEL](/results/v0/RTT_CANCEL_single.png)


**Multi-threaded (4 threads, 50,000 orders):**

- **Throughput:** 5,194 orders/sec
- **RTT stats (ALL):** mean = 721.34μs, p95 = 1,126μs, p99 = 1,601μs
- **NEW orders:** mean = 720.08μs, p95 = 1,133.20μs, p99 = 1,585.88μs
- **MODIFY orders:** mean = 741.44μs, p95 = 1,111μs, p99 = 1,636.16μs
- **CANCEL orders:** mean = 690.06μs, p95 = 1,097.35μs, p99 = 1,566.40μs

![RTT Histogram 4 threads](/results/v0/RTT_ALL_4threads.png)
![RTT Histogram NEW](/results/v0/RTT_NEW_4threads.png)
![RTT Histogram MODIFY](/results/v0/RTT_MODIFY_4threads.png)
![RTT Histogram CANCEL](/results/v0/RTT_CANCEL_4threads.png)


![V0 CPU Profiler Flamegraph](/results/v0/flamegraph.png)

![V0 CPU Profiler Latency](/results/v0/latency.png)

| Function                                 | % CPU | Notes                                        |
| ---------------------------------------- | ----- | -------------------------------------------- |
| `lck_mtx_sleep`                          | 76.7% | Threads blocked on mutexes / lock contention |
| JSON parsing / serialization             | <1%   | Minor CPU usage                              |
| `tiny_malloc_should_clear` / `free_tiny` | ~1%   | Minimal memory overhead                      |


#### **Summary & Analysis**

- **Single-threaded performance** achieves lower latency and more consistent RTTs, but throughput is limited by lack of parallelism.
- **Multi-threaded mode** nearly doubles throughput, but increases mean and tail latencies (p95/p99), likely due to lock contention and resource sharing.
- **NEW, MODIFY, and CANCEL** order types have similar latency profiles, with CANCEL being slightly faster on average.
- **RTT histograms** show a tight distribution for single-threaded, and a wider spread with higher outliers for multi-threaded, indicating more variability under concurrency.
- **CPU profiling** reveals that the majority of CPU time is spent waiting on mutexes (`lck_mtx_sleep`), not on actual order processing or JSON parsing.
- **Optimization opportunities:**  
  - Reduce lock contention and improve data structure efficiency for better multi-threaded scaling.
  - Profile and optimize critical paths in order processing and matching.

*These results provide a baseline for future optimizations and highlight the tradeoff between throughput and latency as concurrency increases.*

---

## v1: Replace Mutex Locks with Faster Alternatives
- TODO: Find a faster locking mechanism (e.g. spinlocks, reader-writer locks) or use lock-free data structures.
- Measure impact on latency and throughput. 
- Analyze CPU profiling to see if lock contention is reduced.

---

## References

### C++ Resources
- [C++ Design Patterns for Low-Latency Applications including High-Frequency Trading](https://arxiv.org/pdf/2309.04259)
- Effective Modern C++ by Scott Meyers

### Data Sources
- [Binance WebSocket API Documentation](https://developers.binance.com/docs/binance-spot-api-docs/web-socket-streams)
- [Binance How to manage a local order book correctly](https://developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/How-to-manage-a-local-order-book-correctly)
- [WebSocket API: Order Book](https://developers.binance.com/docs/derivatives/usds-margined-futures/market-data/websocket-api)

### Order Book Implementations
- Market Microstructure Theory, by Maureen O'Hara
- [OrderBook Repository by TheCodingJesus](https://github.com/Tzadiko/Orderbook/tree/master)

---

## TODO
- Implement v1 optimizations and benchmark.
  - Replace mutexes with faster alternatives.

- Implement v2 optimizations and benchmark.
  - Optimize data structures for order storage and matching.
  - Explore memory pool allocators to reduce allocation overhead.
  - Optimize JSON parsing/serialization if it becomes a bottleneck.