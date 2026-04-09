#pragma once

#include "Order.h"

class OrderModify
{
public:
    OrderModify(OrderId orderId, Price price, Quantity quantity, Side side, SymbolId symbolId)
    : orderId_{ orderId }
    , price_{ price }
    , quantity_{ quantity }
    , side_{ side }
    , symbolId_{ symbolId }
    { }

    OrderId getOrderId() const { return orderId_; }
    Price getPrice() const { return price_; }
    Quantity getQuantity() const { return quantity_; }
    Side getSide() const { return side_; }
    SymbolId getSymbolId() const { return symbolId_; }

    OrderPointer toOrderPointer() const 
    {
        return std::make_shared<Order>(
            getOrderId(),
            getPrice(),
            getQuantity(),
            getSide(),
            getSymbolId()
        );
    }
private:
    OrderId orderId_;
    Price price_;
    Quantity quantity_;
    Side side_;
    SymbolId symbolId_;
};