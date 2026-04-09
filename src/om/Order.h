#pragma once

#include <list>

#include "Usings.h"
#include "Side.h"

class Order 
{
public:
    Order(
        OrderId id,
        Price price,
        Quantity quantity,
        Side side,
        SymbolId symbolId
    )
    : orderId_{id}
    , price_{price}
    , quantity_{quantity}
    , unfilledQuantity_{quantity}
    , side_{side}
    , symbolId_{symbolId}
    { }

    OrderId getOrderId() const { return orderId_; } 
    Price getPrice() const { return price_; }
    Quantity getQuantity() const { return quantity_; }
    Quantity getUnfilledQuantity() const { return unfilledQuantity_; }
    Side getSide() const { return side_; }
    SymbolId getSymbolId() const { return symbolId_; }

    bool isFilled() const { return unfilledQuantity_ == 0; }
    void fill(Quantity qty) { 
        if (qty > unfilledQuantity_) {
            return; // Or throw an exception
        }
        unfilledQuantity_ -= qty; 
    }


private:
    OrderId orderId_;
    Price price_;
    Quantity quantity_;
    Quantity unfilledQuantity_;
    Side side_;
    SymbolId symbolId_;
};


using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::list<OrderPointer>;
