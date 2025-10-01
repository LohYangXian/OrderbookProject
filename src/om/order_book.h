#pragma once

#include <map>
#include <list>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <format>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

using Price = uint32_t; // Price in cents to avoid floating point issues
using Quantity = uint32_t; // Quantity in smallest units (e.g., 0.001 BTC)
using OrderId = uint64_t; // Unique order identifier

enum class Side { BUY, SELL };


class Order 
{
public:
    Order(OrderId id, Price price, Quantity quantity, Side side)
    : orderId_{id}
    , price_{price}
    , quantity_{quantity}
    , unfilledQuantity_{quantity}
    , side_{side} 
    { }

    OrderId getOrderId() const { return orderId_; } 
    Price getPrice() const { return price_; }
    Quantity getQuantity() const { return quantity_; }
    Quantity getUnfilledQuantity() const { return unfilledQuantity_; }
    Side getSide() const { return side_; }

    bool isFilled() const { return unfilledQuantity_ == 0; }
    void fill(Quantity qty) { 
        if (qty > unfilledQuantity_) {
            throw std::logic_error(std::format("Order ({}) cannot be filled for more than its remaining quantity.", getOrderId()));
        }
        unfilledQuantity_ -= qty; 
    }


private:
    OrderId orderId_;
    Price price_;
    Quantity quantity_;
    Quantity unfilledQuantity_;
    Side side_;
};

using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::list<OrderPointer>;

struct TradeInfo
{
    Price price_;
    Quantity quantity_;
    OrderId orderId_;
};

class Trade
{
public:
    Trade(const TradeInfo& bidTrade, const TradeInfo& askTrade)
    : bidTrade_{ bidTrade }
    , askTrade_{ askTrade }
    { }

    const TradeInfo& getBidTradeInfo() const { return bidTrade_; }
    const TradeInfo& getAskTradeInfo() const { return askTrade_; }

private:
    TradeInfo bidTrade_;
    TradeInfo askTrade_;
};


class OrderBook 
{
private:
    std::map<Price, OrderPointers, std::greater<Price>> bids; // Price -> Queue of Orders
    std::map<Price, OrderPointers, std::less<Price>> asks; // Price -> Queue of Orders

public:

    // For this specific Binance code, we should refactor it into a separate binance order book class
    // that inherits from OrderBook and implements processMessage
    void processBinanceMessage(const std::string& message)
    {
        json j = json::parse(message);

        // Update bids
        for (const auto& bid : j["b"]) {
            Price price = static_cast<Price>(std::stod(bid[0].get<std::string>()) * 100);
            Quantity qty = static_cast<Quantity>(std::stod(bid[1].get<std::string>()) * 1000);
            if (qty == 0) {
                bids.erase(price);
            } else {
                // Replace aggregate at price with a single synthetic order
                bids[price].clear();
                bids[price].push_back(std::make_shared<Order>(0, price, qty, Side::BUY));
            }
        }

        // Update asks
        for (const auto& ask : j["a"]) {
            Price price = static_cast<Price>(std::stod(ask[0].get<std::string>()) * 100);
            Quantity qty = static_cast<Quantity>(std::stod(ask[1].get<std::string>()) * 1000);
            if (qty == 0) {
                asks.erase(price);
            } else {
                asks[price].clear();
                asks[price].push_back(std::make_shared<Order>(0, price, qty, Side::SELL));
            }
        }
    };

    void addOrder(const OrderPointer& order)
    {
        if (order->getSide() == Side::BUY) {
            bids[order->getPrice()].push_back(order);
        } else {
            asks[order->getPrice()].push_back(order);
        }

        matchOrders();
    };
    // void cancelOrder(OrderId orderId);
    // void modifyOrder(OrderId orderId, Quantity newQuantity);
    void matchOrders() {
        while (!bids.empty() && !asks.empty()) {
            auto bestBidIt = bids.begin();
            auto bestAskIt = asks.begin();

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
                Trade{TradeInfo{bestBidPrice, tradeQty, bidOrder->getOrderId()},
                      TradeInfo{bestAskPrice, tradeQty, askOrder->getOrderId()}};
                // In a real system, we would store or process the trade further
                std::cout << std::format("Trade executed: {} @ ${:.2f} between Bid Order ID {} and Ask Order ID {}\n",
                                         tradeQty / 1000.00 , bestAskPrice / 100.0, bidOrder->getOrderId(), askOrder->getOrderId());

                if (bidOrder->isFilled()) {
                    bidQueue.pop_front();
                    if (bidQueue.empty()) {
                        bids.erase(bestBidIt);
                    }
                }

                if (askOrder->isFilled()) {
                    askQueue.pop_front();
                    if (askQueue.empty()) {
                        asks.erase(bestAskIt);
                    }
                }
            } else {
                break; // No more matches possible
            }
        }
    };

    void printOrderBook() const
    {
        std::cout << "Order Book:\n";
        std::cout << "Bids:\n";
        for (const auto& [price, orders] : bids) {
            Quantity totalQty = 0;
            for (const auto& order : orders) {
                totalQty += order->getUnfilledQuantity();
            }
            std::cout << std::format("Price: ${:.2f}, Total Quantity: {}\n", price / 100.0, totalQty);
        }

        std::cout << "Asks:\n";
        for (const auto& [price, orders] : asks) {
            Quantity totalQty = 0;
            for (const auto& order : orders) {
                totalQty += order->getUnfilledQuantity();
            }
            std::cout << std::format("Price: ${:.2f}, Total Quantity: {}\n", price / 100.0, totalQty);
        }
    };
};

