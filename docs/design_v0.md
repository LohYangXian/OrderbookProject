# Project Plan: Generic JSON Order Book Engine Simulation & Benchmarking

## Overview

This project aims to build and benchmark a minimal, high-performance order book engine that parses and processes **generic JSON orders**. The workflow involves maintaining a local order book, injecting synthetic or client-provided orders, matching trades, and measuring engine performance under controlled conditions.

---

## Workflow

1. **Generic JSON Order Parsing**
   - Accept JSON messages representing orders (e.g., `{ 
   "OrderId": 123, "Pair": "BTC/USDT",
   "Side": "BUY", "Price": 100.0, "Quantity": 5.0 }`).
   - Parse and validate each order, converting fields to internal types.

2. **Order Book Maintenance**
   - Maintain two structures:
     - **Bids:** Highest price first (max-heap or `std::map` with `std::greater`)
     - **Asks:** Lowest price first (min-heap or `std::map` with `std::less`)
   - Insert new orders, update quantities, and remove filled/cancelled orders.

3. **Order Matching**
   - Apply price-time priority matching:
     - If buy price >= best ask, execute trade at best ask.
     - If sell price <= best bid, execute trade at best bid.
     - Otherwise, add order to the book.
   - Return trade result or acknowledgment as a JSON response.

4. **Synthetic Order Injection & Simulation**
   - Optionally, inject synthetic orders for stress testing and benchmarking.
   - Run large-scale simulations (e.g., 1 million orders) to measure performance.

5. **Performance Measurement**
   - Measure and record:
     - Latency (time to process/match each order)
     - Round trip time for order acknowledgment
   - Aggregate and analyze results.

6. **Reporting**
   - Summarize simulation results and performance metrics.
   - Document design decisions, optimizations, and trade-offs.

---

## Goals

- Build a minimal, robust order book engine for generic JSON orders.
- Support flexible order formats for integration with various clients.
- Benchmark matching speed and latency under different scenarios.
- Provide clear documentation and reproducible results.

---

## Performance Metrics

To evaluate the order book engine and client-server system, the following metrics will be tracked and analyzed:

### 1. **Round Trip Time (RTT) per Order**
- Measure the time from when a client sends an order to when it receives a response from the server.
- Provides insight into end-to-end latency, including network and processing delays.

### 2. **Orders Per Second (Throughput)**
- Track the number of orders processed and acknowledged by the server per second.
- Indicates system scalability and real-time processing capability.

### 3. **Function-Level Latency**
- Measure the time spent in key server-side functions:
  - `addOrder`: Time to insert and process a new order.
  - `matchTrades`: Time to match and execute trades.
- Useful for identifying bottlenecks in the matching engine.
- Will add more functions in the future as they are implemented.

### 4. **Latency Percentiles for Round Trip Time (RTT)**
- Compute and plot latency percentiles:
  - Median (p50)
  - p95
  - p99
  - p99.9
- Visualize tail latency and consistency of order processing.

### 5. **Resource Usage**
- Monitor server CPU and memory usage during benchmarking.
- Helps assess efficiency and identify resource constraints.

### 6. **Dropped or Failed Orders**
- Log any orders that fail to process or are dropped due to errors.
- Indicates reliability and robustness of the system.
---

## Visualization & Analysis

- Plot histograms and CDFs of RTTs and function-level latencies.
- Summarize throughput and resource usage in tables or charts.
- Analyze and report on tail latency (p95, p99, p99.9) for system responsiveness.

---
