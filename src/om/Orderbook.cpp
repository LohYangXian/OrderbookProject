#include "Orderbook.h"

#include <charconv>
#include <string_view>
#include <system_error>

using namespace std;

namespace FixTag {
    constexpr const char* BeginString = "8";
    constexpr const char* MsgType     = "35";
    constexpr const char* OrderId     = "11";
    constexpr const char* Symbol      = "55";
    constexpr const char* Side        = "54";
    constexpr const char* Price       = "44";
    constexpr const char* Quantity    = "38";
}

namespace {

constexpr const char* kOkResponse = "OK";
constexpr const char* kErrResponse = "ERR";
constexpr const char* kCreatedPrefix = "ID:";

struct ParsedFixFields {
    std::string_view msgType;
    std::string_view orderId;
    std::string_view symbol;
    std::string_view side;
    std::string_view price;
    std::string_view quantity;
};

bool parseFixFields(const std::string& message, ParsedFixFields& out)
{
    if (message.rfind("8=FIX.4.2|", 0) != 0) {
        return false;
    }

    size_t start = 0;
    while (start < message.size()) {
        size_t end = message.find('|', start);
        if (end == std::string::npos) {
            end = message.size();
        }

        if (end > start) {
            size_t sep = message.find('=', start);
            if (sep != std::string::npos && sep > start && sep < end) {
                const std::string_view tag(message.data() + start, sep - start);
                const std::string_view value(message.data() + sep + 1, end - sep - 1);

                if (tag == FixTag::MsgType) {
                    out.msgType = value;
                } else if (tag == FixTag::OrderId) {
                    out.orderId = value;
                } else if (tag == FixTag::Symbol) {
                    out.symbol = value;
                } else if (tag == FixTag::Side) {
                    out.side = value;
                } else if (tag == FixTag::Price) {
                    out.price = value;
                } else if (tag == FixTag::Quantity) {
                    out.quantity = value;
                }
            }
        }

        start = end + 1;
    }

    return !out.msgType.empty();
}

template <typename T>
bool parseInteger(std::string_view text, T& out)
{
    if (text.empty()) {
        return false;
    }

    const char* begin = text.data();
    const char* end = begin + text.size();
    const auto [ptr, ec] = std::from_chars(begin, end, out);
    return ec == std::errc() && ptr == end;
}

bool parseSide(std::string_view field, Side& side)
{
    if (field == "1") {
        side = Side::BUY;
        return true;
    }
    if (field == "2") {
        side = Side::SELL;
        return true;
    }
    return false;
}

} // namespace

string Orderbook::processFixMessage(const string& message)
{
    ParsedFixFields fields;
    if (!parseFixFields(message, fields)) {
        return kErrResponse;
    }

    // Step 0: Check for Type field
    // Cancel Type = F, Modify Type = G, New Order Type = D
    // Verify Cancel and Modify messages have the required fields, then process them accordingly
    if (fields.msgType == "F") {
        if (fields.orderId.empty()) {
            return kErrResponse;
        }

        OrderId orderId = 0;
        if (!parseInteger(fields.orderId, orderId)) {
            return kErrResponse;
        }

        cancelOrder(orderId);
        return kOkResponse;
    } else if (fields.msgType == "G") {
        if (fields.orderId.empty() || fields.price.empty() || fields.quantity.empty() ||
            fields.side.empty() || fields.symbol.empty()) {
            return kErrResponse;
        }

        OrderId orderId = 0;
        Price price = 0;
        Quantity qty = 0;
        if (!parseInteger(fields.orderId, orderId) ||
            !parseInteger(fields.price, price) ||
            !parseInteger(fields.quantity, qty)) {
            return kErrResponse;
        }

        Side side;
        if (!parseSide(fields.side, side)) {
            return kErrResponse;
        }

        Symbol symbol(fields.symbol);
        if (qty <= 0) {
            return kErrResponse;
        }
        if (price <= 0) {
            return kErrResponse;
        }

        modifyOrder(OrderModify{orderId, price, qty, side, symbol});
        return kOkResponse;
    } else if (fields.msgType != "D") {
        return kErrResponse;
    } 

    // Step 1: Validate the message structure
    if (fields.orderId.empty() || fields.symbol.empty() || fields.price.empty() ||
        fields.quantity.empty() || fields.side.empty()) {
        return kErrResponse;
    }

    // Step 2: Parse the message
    OrderId orderId = 0;
    Price price = 0;
    Quantity qty = 0;
    if (!parseInteger(fields.orderId, orderId) ||
        !parseInteger(fields.price, price) ||
        !parseInteger(fields.quantity, qty)) {
        return kErrResponse;
    }

    Side side;
    if (!parseSide(fields.side, side)) {
        return kErrResponse;
    }

    Symbol symbol(fields.symbol);
    if (qty <= 0) {
        return kErrResponse;
    }
    if (price <= 0) {
        return kErrResponse;
    }

    {
        std::scoped_lock lock(ordersMutex_);
        if (orderToBook_.find(orderId) != orderToBook_.end()) {
            return kErrResponse;
        }
    }

    // Step 3: Create and add the order
    auto order = std::make_shared<Order>(
        Order(
            orderId,
            price,
            qty,
            side,
            symbol
        )
    );
    
    addOrder(order);
    return std::string(kCreatedPrefix) + std::to_string(orderId);
}



Trades Orderbook::addOrder(const OrderPointer& order)
{
    if (!order || order->getSymbol().empty()) {
        return { };
    }

    std::scoped_lock lock(ordersMutex_);

    if (orderToBook_.find(order->getOrderId()) != orderToBook_.end()) {
        return { };
    }

    auto& book = books_[order->getSymbol()];
    OrderPointers::iterator iterator;

    if (order->getSide() == Side::BUY) {
        auto& orders = book.bids_[order->getPrice()];
        orders.push_back(order);
        iterator = std::prev(orders.end());
    } else {
        auto& orders = book.asks_[order->getPrice()];
        orders.push_back(order);
        iterator = std::prev(orders.end());
    }

    book.orders_.insert({order->getOrderId(), OrderEntry{order, iterator}});
    orderToBook_.insert({order->getOrderId(), &book});

    return matchOrders(book);
}

void Orderbook::cancelOrder(OrderId orderId)
{
    std::scoped_lock lock(ordersMutex_);

    auto bookPtrIt = orderToBook_.find(orderId);
    if (bookPtrIt == orderToBook_.end() || bookPtrIt->second == nullptr) {
        return;
    }

    auto& book = *bookPtrIt->second;

    auto orderIt = book.orders_.find(orderId);
    if (orderIt == book.orders_.end()) {
        orderToBook_.erase(bookPtrIt);
        return;
    }

    const auto [order, iterator] = orderIt->second;
    book.orders_.erase(orderIt);
    orderToBook_.erase(bookPtrIt);

    if (order->getSide() == Side::BUY) {
        auto price = order->getPrice();
        auto& orders = book.bids_.at(price);
        orders.erase(iterator);
        if (orders.empty()) {
            book.bids_.erase(price);
        }
    } else {
        auto price = order->getPrice();
        auto& orders = book.asks_.at(price);
        orders.erase(iterator);
        if (orders.empty()) {
            book.asks_.erase(price);
        }
    }
}

Trades Orderbook::modifyOrder(OrderModify order)
{
    SymbolBook* book = nullptr;
    Symbol existingSymbol;
    {
        std::scoped_lock lock(ordersMutex_);
        auto bookPtrIt = orderToBook_.find(order.getOrderId());
        if (bookPtrIt == orderToBook_.end() || bookPtrIt->second == nullptr) {
            return { };
        }

        book = bookPtrIt->second;
        auto orderIt = book->orders_.find(order.getOrderId());
        if (orderIt == book->orders_.end()) {
            orderToBook_.erase(bookPtrIt);
            return { };
        }
        existingSymbol = orderIt->second.order_->getSymbol();
    }

    // Keep modification symbol-scoped to avoid moving an order across books implicitly.
    if (existingSymbol != order.getSymbol()) {
        return { };
    }

    cancelOrder(order.getOrderId());
    return addOrder(order.toOrderPointer());
}

Trades Orderbook::matchOrders(SymbolBook& book)
{
    Trades trades;
    trades.reserve(book.bids_.size() + book.asks_.size());

    while (!book.bids_.empty() && !book.asks_.empty()) {
        auto bestBidIt = book.bids_.begin();
        auto bestAskIt = book.asks_.begin();

        Price bestBidPrice = bestBidIt->first;
        Price bestAskPrice = bestAskIt->first;

        if (bestBidPrice < bestAskPrice) {
            break;
        }

        auto& bidQueue = bestBidIt->second;
        auto& askQueue = bestAskIt->second;

        auto bidOrder = bidQueue.front();
        auto askOrder = askQueue.front();

        Quantity tradeQty = std::min(bidOrder->getUnfilledQuantity(), askOrder->getUnfilledQuantity());

        bidOrder->fill(tradeQty);
        askOrder->fill(tradeQty);

        trades.push_back(Trade{
            TradeInfo{bestBidPrice, tradeQty, bidOrder->getOrderId(), bidOrder->getSymbol()},
            TradeInfo{bestAskPrice, tradeQty, askOrder->getOrderId(), askOrder->getSymbol()}
        });

        if (bidOrder->isFilled()) {
            bidQueue.pop_front();
            book.orders_.erase(bidOrder->getOrderId());
            orderToBook_.erase(bidOrder->getOrderId());
        }

        if (askOrder->isFilled()) {
            askQueue.pop_front();
            book.orders_.erase(askOrder->getOrderId());
            orderToBook_.erase(askOrder->getOrderId());
        }

        if (bidQueue.empty()) {
            book.bids_.erase(bestBidIt);
        }
        if (askQueue.empty()) {
            book.asks_.erase(bestAskIt);
        }
    }

    return trades;
}

void Orderbook::printOrderBook() const
{
    std::cout << "Order Book:\n";

    for (const auto& [symbol, book] : books_) {
        std::cout << "Symbol: " << symbol << "\n";
        std::cout << "Bids:\n";
        for (const auto& [price, orders] : book.bids_) {
            Quantity totalQty = 0;
            for (const auto& order : orders) {
                totalQty += order->getUnfilledQuantity();
            }
            std::cout << "Price: $" << price << ", Total Quantity: " << totalQty << "\n";
        }

        std::cout << "Asks:\n";
        for (const auto& [price, orders] : book.asks_) {
            Quantity totalQty = 0;
            for (const auto& order : orders) {
                totalQty += order->getUnfilledQuantity();
            }
            std::cout << "Price: $" << price << ", Total Quantity: " << totalQty << "\n";
        }
    }
}
