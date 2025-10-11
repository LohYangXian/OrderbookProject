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

// Insert design diagram here

Client sends JSON orders to Server -> Server processes orders through Order Book Engine -> Server returns JSON responses indicating trade execution or order addition.

Round Trip Time (RTT) is measured from Client order submission to Server acknowledgment.

Performance metrics such as latency (ns/op), throughput (orders/sec), and memory usage are tracked at each iteration.

---

## Tech Stack

- **Language**: C++20
- **Build**: Bazel
- **Testing/Benchmarking**:
  - Google Benchmark
  - `std::chrono` + `rdtsc`
  - `gperf` for cache misses/branch mispredicts
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

## How to run it

To build the project (manual mode), run:
```bash
bazel build //src:main_manual
```

To run the main manual application:
```bash
bazel run //src:main_manual
```

To build the project (server mode), run:
```bash
bazel build //src:main_server
```

To run the main server application:
```bash
bazel run //src:main_server
```

To run the tests:
```bash
bazel test //tests:orderbook_test
```

## Using `gperf` to Measure Performance

To measure performance using `gperf`, follow these steps:

1. **Build the Project (Don't use Bazel, brew install needed libraries)** 
    ```bash 
    g++ -std=c++20 -g -I/opt/homebrew/include -Isrc/om -Isrc/server -L/opt/homebrew/lib src/server/Server.cpp src/main_server.cpp src/om/Orderbook.cpp -lprofiler -o main_server
    ```

2. **Start the server with gperf recording:**
    ```bash
    CPUPROFILE=orderbook_server.prof DYLD_INSERT_LIBRARIES=/opt/homebrew/lib/libprofiler.dylib ./main_server
    ```

2. **Run the client to send orders to the server:**
    ```bash
    python py_client/client.py
    ```

3. **Stop the server (Ctrl+C) after the client finishes.**

4. **Generate the perf report:**
    ```bash
    brew install go
    go install github.com/google/pprof@latest
    pprof --text ./bazel-bin/src/main_server orderbook_server.prof
    ```

## Versions of Orderbook Engine

### v0: Baseline Implementations
- Implement basic order book engine with naive data structures (e.g. `std::map`, `std::list`).n
- Support limit orders, order adding, order modifying, order cancelling, order matching and trade generation.
- Basic mutex locking for thread safety.
- Cover workflow from order ingestion to matching to trade generation.
- Measure baseline latency and throughput using `std::chrono`, `rdtsc`, and `gperf`.

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

---

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

---

#### **Summary & Analysis**

- **Single-threaded performance** achieves lower latency and more consistent RTTs, but throughput is limited by lack of parallelism.
- **Multi-threaded mode** nearly doubles throughput, but increases mean and tail latencies (p95/p99), likely due to lock contention and resource sharing.
- **NEW, MODIFY, and CANCEL** order types have similar latency profiles, with CANCEL being slightly faster on average.
- **RTT histograms** show a tight distribution for single-threaded, and a wider spread with higher outliers for multi-threaded, indicating more variability under concurrency.
- **Optimization opportunities:**  
  - Reduce lock contention and improve data structure efficiency for better multi-threaded scaling.
  - Profile and optimize critical paths in order processing and matching.

*These results provide a baseline for future optimizations and highlight the tradeoff between throughput and latency as concurrency increases.*

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


## TODO:
- Fix the gPerf instructions (they are broken)
- After fixing gPerf instructions, add gPerf results for v0
- Add more profiling results (cache misses, branch mispredicts, etc)
- Add design diagrams
- After v0 is done, add v1, v2, etc with detailed changes and results