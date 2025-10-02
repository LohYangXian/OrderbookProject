#pragma once

#include <map>
#include <iostream>
#include <mutex>

#include "Usings.h"
#include "Side.h"
#include "Order.h"
#include "TradeInfo.h"
#include "Trade.h"

class Orderbook 
{
private:
    std::map<Price, OrderPointers, std::greater<Price>> bids_; // Price -> Queue of Orders
    std::map<Price, OrderPointers, std::less<Price>> asks_; // Price -> Queue of Orders
    mutable std::mutex ordersMutex_;

public:

    // For this specific Binance code, we should refactor it into a separate binance order book class
    // that inherits from OrderBook and implements processMessage
    void processBinanceMessage(const std::string& message);
    
    // This will be our v0 generic message processor
    // We will use a server to receive messages and call this function
    // then we will test the performance of this function
    json processJsonMessage(const json& message);

    Trades addOrder(const OrderPointer& order);
   
    // void cancelOrder(OrderId orderId);
    // void modifyOrder(OrderId orderId, Quantity newQuantity);
    
    Trades matchOrders();

    void printOrderBook() const;
};

