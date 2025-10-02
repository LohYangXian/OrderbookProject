#pragma once

#include <list>
#include <format>

#include "Usings.h"
#include "Side.h"

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
