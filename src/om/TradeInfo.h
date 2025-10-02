#pragma once

#include "Usings.h"

struct TradeInfo
{
    Price price_;
    Quantity quantity_;
    OrderId orderId_;

    Price getPrice() const { return price_; }
    Quantity getQuantity() const { return quantity_; }
    OrderId getOrderId() const { return orderId_; }
};