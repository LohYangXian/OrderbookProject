#pragma once

#include <map>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_map>

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

    struct SymbolBook {
        std::map<Price, OrderPointers, std::greater<Price>> bids_;
        std::map<Price, OrderPointers, std::less<Price>> asks_;
        std::unordered_map<OrderId, OrderEntry> orders_;
    };

    std::unordered_map<Symbol, SymbolBook> books_; // Symbol -> independent order book
    std::unordered_map<OrderId, SymbolBook*> orderToBook_; // OrderId -> owning symbol book
    mutable std::mutex ordersMutex_;

    Trades matchOrders(SymbolBook& book);

public:

    // For this specific Binance code, we should refactor it into a separate binance order book class
    // that inherits from OrderBook and implements processMessage
    void processBinanceMessage(const std::string& message);
    
    // Process simplified FIX messages (tag=value|tag=value|...)
    std::string processFixMessage(const std::string& message);

    Trades addOrder(const OrderPointer& order);
   
    void cancelOrder(OrderId orderId);
    Trades modifyOrder(OrderModify order);
    
    void printOrderBook() const;

    // For testing purposes
    const auto& getBids(const Symbol& symbol) const { return books_.at(symbol).bids_; }
    const auto& getAsks(const Symbol& symbol) const { return books_.at(symbol).asks_; }
};

