# Project Plan: Binance Order Book Engine Simulation & Benchmarking

## Overview

This project aims to build and benchmark a high-performance order book engine using real-time Binance WebSocket data. The workflow involves maintaining a local order book, injecting synthetic orders for matching, and measuring engine performance under realistic market conditions.

---

## Workflow

1. **Maintain Local Order Book**
   - Subscribe to Binance’s WebSocket depth stream.
   - Continuously update a local order book with incoming price/quantity changes.

2. **Synthetic Order Injection**
   - In a separate thread, inject synthetic buy/sell orders into the engine.
   - Orders are matched against the current state of the local order book.

3. **Performance Measurement**
   - Measure key metrics for each synthetic order:
     - Latency (time to match/fill)
     - CPU cycles consumed
     - Throughput (orders processed per second)
   - Record results for each simulation run.

4. **Optimization Iterations**
   - After each simulation, optimize a specific aspect of the engine (e.g., data structure, memory management, threading).
   - Rerun the simulation and recompute performance metrics.
   - Repeat for multiple optimization strategies.

5. **Reporting**
   - Summarize results from all iterations.
   - Compare performance improvements and trade-offs.
   - Present findings in a final report.

---

## Goals

- Build a robust, real-time order book engine using Binance market data.
- Simulate realistic order matching and trade generation.
- Benchmark engine performance and identify bottlenecks.
- Experiment with optimizations and quantify their impact.
- Provide clear documentation and a summary report of all findings.

---

## Deliverables

- Source code for the order book engine and benchmarking framework.
- Scripts for running simulations and collecting metrics.
- Documentation of design decisions and optimization strategies.
- Final report summarizing results and recommendations.
