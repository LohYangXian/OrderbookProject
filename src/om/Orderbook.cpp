#include "Orderbook.h"

//For this our sample structure looks like this:
//{
// "Type": "NEW" // or "CANCEL" or "MODIFY",
// "OrderId": 123456,
// "Pair": "BTCUSDT",
// "Price": "10500.00",
// "Quantity": "0.005",
// "Side": "BUY"  // or "SELL"
//}
// We will parse this JSON and create an Order object
//TODO: Improve this, refactor, add error handling, logging, etc.
json Orderbook::processJsonMessage(const json& message)
{
    // Step 0: Check for Type field
    if (!message.contains("Type")) {
        return json{{"status", "error"}, {"message", "Missing Type field"}};
    }
    std::string type = message["Type"];
    if (type == "CANCEL") {
        if (!message.contains("OrderId")) {
            return json{{"status", "error"}, {"message", "Missing OrderId for CANCEL"}};
        }
        OrderId orderId = static_cast<OrderId>(message["OrderId"]);
        cancelOrder(orderId);
        return json{{"status", "success"}, {"message", "Order cancelled"}};
    } else if (type == "MODIFY") {
        if (!message.contains("OrderId") || !message.contains("Price") || 
            !message.contains("Quantity") || !message.contains("Side")
            || !message.contains("Pair")) {
            return json{{"status", "error"}, {"message", "Invalid MODIFY message format"}};
        }
        OrderId orderId = static_cast<OrderId>(message["OrderId"]);
        Price price = static_cast<Price>(std::stod(message["Price"].get<std::string>()));
        Quantity qty = static_cast<Quantity>(std::stod(message["Quantity"].get<std::string>()));
        Side side;
        if (message["Side"] == "BUY") {
            side = Side::BUY;
        } else if (message["Side"] == "SELL") {
            side = Side::SELL;
        } else {
            return json{{"status", "error"}, {"message", "Invalid side value"}};
        }
        if (qty <= 0) {
            return json{{"status", "error"}, {"message", "Quantity cannot be zero or negative"}};
        }
        if (price <= 0) {
            return json{{"status", "error"}, {"message", "Price cannot be zero or negative"}};
        }

        auto trades = modifyOrder(OrderModify{orderId, price, qty, side});
        if (!trades.empty()) {
            json tradesJson = json::array();
            for (const auto& trade : trades) {
                tradesJson.push_back({
                    {"bidOrderId", trade.getBidTradeInfo().getOrderId()},
                    {"price", trade.getBidTradeInfo().getPrice()}, // Use bid price for trade price
                    {"quantity", trade.getBidTradeInfo().getQuantity()}, // Quantity is the same for both sides
                    {"askOrderId", trade.getAskTradeInfo().getOrderId()},
                    {"price", trade.getAskTradeInfo().getPrice()}, // Use ask price for trade price
                    {"quantity", trade.getAskTradeInfo().getQuantity()} // Quantity is the same for both sides
                });
            }
            return json{{"status", "success"}, {"modified trades executed", tradesJson}};
        } else {
            return json{{"status", "success"}, {"message", "Order modified, no trades executed"}};
        } 
    } else if (type != "NEW") {
        return json{{"status", "error"}, {"message", "Invalid Type value"}};
    } 

    // Step 1: Validate the message structure
    if (!message.contains("OrderId") || !message.contains("Pair") || !message.contains("Price") || 
        !message.contains("Quantity") || !message.contains("Side")) {
        return json{{"status", "error"}, {"message", "Invalid message format"}};
    }

    // Step 2: Parse the message
    OrderId orderId = static_cast<OrderId>(message["OrderId"]);
    Price price = static_cast<Price>(std::stod(message["Price"].get<std::string>()));
    Quantity qty = static_cast<Quantity>(std::stod(message["Quantity"].get<std::string>()));
    Side side;
    if (message["Side"] == "BUY") {
        side = Side::BUY;
    } else if (message["Side"] == "SELL") {
        side = Side::SELL;
    } else {
        return json{{"status", "error"}, {"message", "Invalid side value"}};
    }
    if (qty <= 0) {
        return json{{"status", "error"}, {"message", "Quantity cannot be zero or negative"}};
    }
    if (price <= 0) {
        return json{{"status", "error"}, {"message", "Price cannot be zero or negative"}};
    }

    // Step 3: Create and add the order
    auto order = std::make_shared<Order>(
        Order(
            orderId,
            price,
            qty,
            side
        )
    );
    
    auto trades = addOrder(order);
    if (!trades.empty()) {
        json tradesJson = json::array();
        for (const auto& trade : trades) {
            tradesJson.push_back({
                {"bidOrderId", trade.getBidTradeInfo().getOrderId()},
                {"price", trade.getBidTradeInfo().getPrice()}, // Use bid price for trade price
                {"quantity", trade.getBidTradeInfo().getQuantity()}, // Quantity is the same for both sides
                {"askOrderId", trade.getAskTradeInfo().getOrderId()},
                {"price", trade.getAskTradeInfo().getPrice()}, // Use ask price for trade price
                {"quantity", trade.getAskTradeInfo().getQuantity()} // Quantity is the same for both sides
            });
        }
        return json{{"status", "success"}, {"trades", tradesJson}};
    } else {
        return json{{"status", "success"}, {"message", "Order added, no trades executed"}};
    } 
}

void Orderbook::processBinanceMessage(const std::string& message)
{
    json j = json::parse(message);

    // Update bids
    for (const auto& bid : j["b"]) {
        Price price = static_cast<Price>(std::stod(bid[0].get<std::string>()) * 100);
        Quantity qty = static_cast<Quantity>(std::stod(bid[1].get<std::string>()) * 1000);
        if (qty == 0) {
            bids_.erase(price);
        } else {
            // Replace aggregate at price with a single synthetic order
            bids_[price].clear();
            bids_[price].push_back(std::make_shared<Order>(0, price, qty, Side::BUY));
        }
    }

    // Update asks
    for (const auto& ask : j["a"]) {
        Price price = static_cast<Price>(std::stod(ask[0].get<std::string>()) * 100);
        Quantity qty = static_cast<Quantity>(std::stod(ask[1].get<std::string>()) * 1000);
        if (qty == 0) {
            asks_.erase(price);
        } else {
            asks_[price].clear();
            asks_[price].push_back(std::make_shared<Order>(0, price, qty, Side::SELL));
        }
    }
};

Trades Orderbook::addOrder(const OrderPointer& order)
{

    std::scoped_lock lock(ordersMutex_);

    if (orders_.contains(order->getOrderId())) {
        return { }; // Duplicate order ID
    }

    OrderPointers::iterator iterator;

    if (order->getSide() == Side::BUY) {
        auto& orders = bids_[order->getPrice()];
        orders.push_back(order);
        iterator = std::prev(orders.end());
    } else {
        auto& orders = asks_[order->getPrice()];
        orders.push_back(order);
        iterator = std::prev(orders.end());
    }

    orders_.insert({order->getOrderId(), OrderEntry{order, iterator}});

    return matchOrders();
};


void Orderbook::cancelOrder(OrderId orderId)
{

    std::scoped_lock lock(ordersMutex_);

    if (!orders_.contains(orderId)) {
        return; // Order not found
    }

    const auto [order, iterator] = orders_[orderId];
    orders_.erase(orderId);

    if (order->getSide() == Side::BUY) {
        auto price = order->getPrice();
        auto& orders = bids_.at(price);
        orders.erase(iterator);
        if (orders.empty()) {
            bids_.erase(price);
        }
    } else {
        auto price = order->getPrice();
        auto& orders = asks_.at(price);
        orders.erase(iterator);
        if (orders.empty()) {
            asks_.erase(price);
        }
    }
};

Trades Orderbook::modifyOrder(OrderModify order)
{
    {
        std::scoped_lock lock(ordersMutex_);
        if (!orders_.contains(order.getOrderId())) {
            return { }; // Order not found
        }
    }

    cancelOrder(order.getOrderId());
    return addOrder(order.toOrderPointer());
}

Trades Orderbook::matchOrders() {
    Trades trades;
    trades.reserve(bids_.size() + asks_.size()); // Rough estimate

    while (!bids_.empty() && !asks_.empty()) {
        auto bestBidIt = bids_.begin();
        auto bestAskIt = asks_.begin();

        Price bestBidPrice = bestBidIt->first;
        Price bestAskPrice = bestAskIt->first;

        if (bestBidPrice >= bestAskPrice) {
            auto& bidQueue = bestBidIt->second;
            auto& askQueue = bestAskIt->second;

            auto bidOrder = bidQueue.front();
            auto askOrder = askQueue.front();

            Quantity tradeQty = std::min(bidOrder->getUnfilledQuantity(), askOrder->getUnfilledQuantity());

            bidOrder->fill(tradeQty);
            askOrder->fill(tradeQty);

            // Log the trade
            trades.push_back(Trade{TradeInfo{bestBidPrice, tradeQty, bidOrder->getOrderId()},
                    TradeInfo{bestAskPrice, tradeQty, askOrder->getOrderId()}});

            if (bidOrder->isFilled()) {
                bidQueue.pop_front();
                orders_.erase(bidOrder->getOrderId());
            }

            if (askOrder->isFilled()) {
                askQueue.pop_front();
                orders_.erase(askOrder->getOrderId());
            }

            if (bidQueue.empty()) {
                bids_.erase(bestBidIt);
            }
            if (askQueue.empty()) {
                asks_.erase(bestAskIt);
            }
        } else {
            break; // No more matches possible
        }
    }
    return trades;
};

void Orderbook::printOrderBook() const
{
    std::cout << "Order Book:\n";
    std::cout << "Bids:\n";
    for (const auto& [price, orders] : bids_) {
        Quantity totalQty = 0;
        for (const auto& order : orders) {
            totalQty += order->getUnfilledQuantity();
        }
        std::cout << "Price: $" << price << ", Total Quantity: " << totalQty << "\n";
    }

    std::cout << "Asks:\n";
    for (const auto& [price, orders] : asks_) {
        Quantity totalQty = 0;
        for (const auto& order : orders) {
            totalQty += order->getUnfilledQuantity();
        }
        std::cout << "Price: $" << price << ", Total Quantity: " << totalQty << "\n";
    }
}
