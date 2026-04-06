#pragma once

#include <map>
#include <iostream>
#include <mutex>

#include "Usings.h"
#include "Side.h"
#include "Order.h"
#include "TradeInfo.h"
#include "Trade.h"
#include "OrderModify.h"

class Orderbook 
{
private:

    struct OrderEntry {
        OrderPointer order_{ nullptr };
        OrderPointers::iterator location_;
    };

    std::map<Price, OrderPointers, std::greater<Price>> bids_; // Price -> Queue of Orders
    std::map<Price, OrderPointers, std::less<Price>> asks_; // Price -> Queue of Orders
    std::unordered_map<OrderId, OrderEntry> orders_; // OrderId -> OrderEntry for quick lookup, this is to support cancel/modify
    mutable std::mutex ordersMutex_;

    Trades matchOrders();

public:

    // For this specific Binance code, we should refactor it into a separate binance order book class
    // that inherits from OrderBook and implements processMessage
    void processBinanceMessage(const std::string& message);
    
    // This will be our v0 generic message processor
    // We will use a server to receive messages and call this function
    // then we will test the performance of this function
    json processJsonMessage(const json& message);

    Trades addOrder(const OrderPointer& order);
   
    void cancelOrder(OrderId orderId);
    Trades modifyOrder(OrderModify order);
    
    void printOrderBook() const;

    // For testing purposes
    const auto& getBids() const { return bids_; }
    const auto& getAsks() const { return asks_; }
};

