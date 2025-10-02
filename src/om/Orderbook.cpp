#include "Orderbook.h"

//For this our sample structure looks like this:
//{
// "OrderId": 123456,
// "Pair": "BTCUSDT",
// "Price": "10500.00",
// "Quantity": "0.005",
// "Side": "BUY"  // or "SELL"
//}
json Orderbook::processJsonMessage(const json& message)
{
    // Step 1: Validate the message structure
    if (!message.contains("OrderId") || !message.contains("Pair") || !message.contains("Price") || 
        !message.contains("Quantity") || !message.contains("Side")
        || message["Pair"] != "BTC/USDT") {
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
    if (order->getSide() == Side::BUY) {
        bids_[order->getPrice()].push_back(order);
    } else {
        asks_[order->getPrice()].push_back(order);
    }
    return matchOrders();
};


// void cancelOrder(OrderId orderId);
// void modifyOrder(OrderId orderId, Quantity newQuantity);
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
                if (bidQueue.empty()) {
                    bids_.erase(bestBidIt);
                }
            }

            if (askOrder->isFilled()) {
                askQueue.pop_front();
                if (askQueue.empty()) {
                    asks_.erase(bestAskIt);
                }
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
        std::cout << std::format("Price: ${}, Total Quantity: {}\n", price, totalQty);
    }

    std::cout << "Asks:\n";
    for (const auto& [price, orders] : asks_) {
        Quantity totalQty = 0;
        for (const auto& order : orders) {
            totalQty += order->getUnfilledQuantity();
        }
        std::cout << std::format("Price: ${}, Total Quantity: {}\n", price, totalQty);
    }
};
