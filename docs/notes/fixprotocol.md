# FIX Protocol Order Message Documentation

## Overview
The **FIX (Financial Information eXchange) Protocol** is an industry-standard messaging protocol used for real-time electronic trading.  

This document describes a **FIX 4.2 New Order Single (MsgType = D)** message, which is used to submit a new order (e.g. buy/sell) into the market.

---

## Message Structure

A FIX message is composed of a sequence of `tag=value` fields separated by a delimiter (commonly `|` for readability or SOH in production).

Example:
8=FIX.4.2|9=176|35=D|49=ONIXS|56=CME|34=2|11=983532-3|55=NVDA|54=1|38=100|40=1|60=20260407-21:00:00|10=128|


---

## Core Header Fields

- **BeginString (8)**  
  Protocol version identifier (e.g. `FIX.4.2`)

- **BodyLength (9)**  
  Length of the message body (used for validation)

- **MsgType (35)**  
  Message type identifier  
  - `D` = New Order Single

- **MsgSeqNum (34)**  
  Sequence number to ensure ordered delivery

- **SenderCompID (49)**  
  Identifier of the message sender

- **TargetCompID (56)**  
  Identifier of the message receiver

- **CheckSum (10)**  
  Message integrity validation

---

## Order Fields

- **ClOrdID (11)**  
  Unique client order ID for tracking

- **Symbol (55)**  
  Instrument being traded (e.g. `NVDA`)

- **Side (54)**  
  - `1` = Buy  
  - `2` = Sell  

- **OrderQty (38)**  
  Quantity of the order

- **OrdType (40)**  
  - `1` = Market  
  - `2` = Limit  

- **TransactTime (60)**  
  Timestamp of order creation

---

## Why FIX is Used in HFT

### 1. Industry Standard
FIX is universally adopted across exchanges, brokers, and trading firms.  
Using FIX allows direct interoperability without custom protocol translation.

### 2. Deterministic & Structured
The tag-value format ensures:
- Predictable parsing
- Clear field semantics
- Consistent message structure

This is critical in latency-sensitive systems.

### 3. Reliability Guarantees
FIX provides built-in mechanisms such as:
- Sequence numbers (MsgSeqNum)
- Checksums (CheckSum)

These ensure:
- No message loss
- Correct ordering
- Data integrity

### 4. High Throughput
FIX messages are lightweight and optimized for streaming large volumes of orders, making them suitable for high-frequency trading environments.

### 5. Real-World Compatibility
Most real trading systems, gateways, and exchanges natively support FIX, making it essential for production-grade systems.

---

## Usage in Orderbook Project

To simulate realistic trading conditions, our orderbook engine adopts the FIX protocol format for incoming orders.

### Objectives

- **Realism**  
  Aligns the system with real-world trading infrastructure and exchange communication standards.

- **Fair Performance Measurement**  
  Using FIX ensures that:
  - Parsing overhead is accounted for
  - Latency measurements reflect production-like conditions
  - Benchmarks are comparable to real trading systems

- **System Design Fidelity**  
  Enables us to:
  - Model real client/exchange interaction flows
  - Test sequencing, validation, and message handling logic
  - Extend easily to more advanced message types in the future

---

## Summary

The FIX protocol serves as the backbone of modern electronic trading systems.  
By incorporating FIX into our orderbook engine, we ensure that our system is not only performant but also aligned with real-world trading architectures.