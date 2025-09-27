# Design for Version 0

## Data Source
The orderbook will be populated and updated using real-time data from the Binance WebSocket API.

### Sample websocket message payload:

```
{
  "e": "depthUpdate", // Event type
  "E": 123456789,     // Event time
  "T": 123456788,     // Transaction time 
  "s": "BTCUSDT",     // Symbol
  "U": 157,           // First update ID in event
  "u": 160,           // Final update ID in event
  "pu": 149,          // Final update Id in last stream(ie `u` in last stream)
  "b": [              // Bids to be updated
    [
      "0.0024",       // Price level to be updated
      "10"            // Quantity
    ]
  ],
  "a": [              // Asks to be updated
    [
      "0.0026",       // Price level to be updated
      "100"          // Quantity
    ]
  ]
}
```

## Orderbook Design
The goal of the orderbook is to maintain a real-time view of the market depth for a given trading pair (e.g., BTCUSDT). The orderbook will be updated based on incoming websocket messages that provide updates to bid and ask prices and quantities.
An in-memory orderbook will be built and maintained using the incoming websocket messages.

### Assumptions
- The orderbook only supports a **single** trading pair (e.g., BTCUSDT). 
- The orderbook maintains only the top N levels of bids and asks (e.g., top 10 levels).
- The market data does not include order IDs or individual order details, only price levels and quantities. We will not be supporting multiple order types (limit, market, etc.) in this version.
- All orders are assumed to be limit orders for simplicity.
- Validation of update IDs (U, u, pu) is not implemented in this version.
- Bitemporal timestamping and persistence are also not implemented in this version.

### Data Structures


#### Bids
Red-black tree of bid prices and their corresponding quantities.

#### Asks
Red-black tree of ask prices and their corresponding quantities.

#### Order
A structure to represent an individual order with fields for order ID, price, quantity, and side (buy/sell).

```
struct Order {
    uint32_t price;      // Price in cents
    uint32_t quantity;   // Quantity in milli-units
    uint32_t fillQuantity;
    uint64_t orderId;
    bool isBuy;
};
```
##### Primitive Type Rationale
- **price (`uint32_t`)**:  
  Price is stored in cents as an unsigned 32-bit integer. This avoids **floating-point rounding errors** and is sufficient for assets like Bitcoin, where the price in cents will not exceed the maximum value of a 32-bit unsigned integer (4,294,967,295). Using cents as the unit allows for precise representation and fast arithmetic.

- **quantity (`uint32_t`)**:  
  Quantity is stored in milli-units (three significant figures) as an unsigned 32-bit integer. This allows for accurate representation of fractional quantities (e.g., 0.112 BTC is stored as 112). The range of `uint32_t` is more than enough for typical trading quantities, and using an integer type ensures precision and performance.

- **fillQuantity (`uint32_t`)**:  
  Like `quantity`, this tracks the filled portion of an order in milli-units, using an unsigned 32-bit integer for the same reasons.

- **orderId (`uint64_t`)**:  
  Order IDs are stored as unsigned 64-bit integers to support a very large number of unique orders over time, ensuring no risk of overflow. No negative order IDs are expected, so an unsigned type is appropriate.

- **isBuy (`bool`)**:  
  The side of the order (buy or sell) is represented as a boolean for simplicity and efficiency.

#### Order Book

A class that encapsulates the bids and asks data structures and provides basic functions to update and query the order book.
  - Supported operations:
    - Process incoming websocket messages and update the order book.
    - Match orders and generate trades when matching orders are found.
    - Print the current state of the order book for debugging. 

##### Example skeleton code:
```class OrderBook {
public:
    void processMessage(const WebSocketMessage& msg);
    void printOrderBook() const;
private:
    std::map<uint32_t, std::deque<Order>> bids; // Price -> Queue of Orders
    std::map<uint32_t, std::deque<Order>> asks; // Price -> Queue of Orders
};
```

#### Trade
A structure to represent a trade with fields for trade ID, price, quantity, and timestamp.

```
struct TradeInfo {
    OrderId orderId;
    Price price_;
    Quantity quantity_;
};

struct Trade {
    TradeInfo bidTrade;
    TradeInfo askTrade;
};
```


## Future Enhancements
- Use depth request together with websocket updates to initialize and maintain the orderbook. [WebSocket API: Order Book](https://developers.binance.com/docs/derivatives/usds-margined-futures/market-data/websocket-api)
- Implement simple validation on websocket message payloads. [Binance How to manage a local order book correctly](https://developers.binance.com/docs/derivatives/usds-margined-futures/websocket-market-streams/How-to-manage-a-local-order-book-correctly)