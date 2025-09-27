#include <map>
#include <list>
#include <memory>
#include <stdexcept>
#include <format>

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

class OrderBook 
{
private:
    std::map<Price, std::list<OrderPointers>, std::greater<Price>> bids; // Price -> Queue of Orders
    std::map<Price, std::list<OrderPointer>, std::less<Price>> asks; // Price -> Queue of Orders

public:
    void addOrder(const OrderPointer& order);
    // void cancelOrder(OrderId orderId);
    // void modifyOrder(OrderId orderId, Quantity newQuantity);
    void matchOrders();
    void printOrderBook() const;
};

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


