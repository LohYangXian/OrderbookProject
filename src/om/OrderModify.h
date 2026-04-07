#pragma once

#include "Order.h"

class OrderModify
{
public:
    OrderModify(OrderId orderId, Price price, Quantity quantity, Side side, Symbol symbol)
    : orderId_{ orderId }
    , price_{ price }
    , quantity_{ quantity }
    , side_{ side }
    , symbol_{ std::move(symbol) }
    { }

    OrderId getOrderId() const { return orderId_; }
    Price getPrice() const { return price_; }
    Quantity getQuantity() const { return quantity_; }
    Side getSide() const { return side_; }
    Symbol getSymbol() const { return symbol_; }

    OrderPointer toOrderPointer() const 
    {
        return std::make_shared<Order>(
            getOrderId(),
            getPrice(),
            getQuantity(),
            getSide(),
            getSymbol()
        );
    }
private:
    OrderId orderId_;
    Price price_;
    Quantity quantity_;
    Side side_;
    Symbol symbol_;
};