#pragma once

#include <array>
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
#include "OrderPool.h"

class Orderbook 
{
private:
    static constexpr std::size_t kPreallocatedOrderCapacity = 5'000'000;
    static constexpr std::size_t kSymbolCount = static_cast<std::size_t>(kKnownSymbolCount);

    OrderPool orderPool_;

    struct SymbolBook {
        std::map<Price, OrderPointers, std::greater<Price>> bids_;
        std::map<Price, OrderPointers, std::less<Price>> asks_;
    };

    struct OrderLocator {
        OrderPointer order_{ nullptr };
        OrderPointers::iterator location_;
        SymbolBook* book_{ nullptr };
    };

    std::array<SymbolBook, kSymbolCount> books_;
    std::vector<OrderLocator> orderLocators_;
    std::unordered_map<OrderId, OrderLocator> overflowOrderLocators_;
    mutable std::mutex ordersMutex_;

    static bool isKnownSymbol(SymbolId symbolId);
    SymbolBook& symbolBook(SymbolId symbolId);
    const SymbolBook& symbolBook(SymbolId symbolId) const;

    bool hasOrderLocatorUnlocked(OrderId orderId) const;
    OrderLocator* getOrderLocatorUnlocked(OrderId orderId);
    const OrderLocator* getOrderLocatorUnlocked(OrderId orderId) const;
    void upsertOrderLocatorUnlocked(OrderId orderId, OrderLocator locator);
    void eraseOrderLocatorUnlocked(OrderId orderId);

    OrderPointer makePooledOrder(OrderId orderId, Price price, Quantity quantity, Side side, SymbolId symbolId);
    Trades matchOrders(SymbolBook& book);

public:
    Orderbook();

    // For this specific Binance code, we should refactor it into a separate binance order book class
    // that inherits from OrderBook and implements processMessage
    void processBinanceMessage(const std::string& message);
    
    // Process simplified FIX messages (tag=value|tag=value|...)
    std::string processFixMessage(const std::string_view message);

    Trades addOrder(const OrderPointer& order);
   
    void cancelOrder(OrderId orderId);
    Trades modifyOrder(OrderModify order);
    
    void printOrderBook() const;

    // For testing purposes
    const auto& getBids(SymbolId symbolId) const { return symbolBook(symbolId).bids_; }
    const auto& getAsks(SymbolId symbolId) const { return symbolBook(symbolId).asks_; }
};

