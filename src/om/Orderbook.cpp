#include "Orderbook.h"

#include <charconv>
#include <stdexcept>
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
constexpr std::size_t kOrderPoolChunkSize = 4096;

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

bool parseSymbolId(std::string_view field, SymbolId& symbolId)
{
    symbolId = toSymbolId(field);
    return symbolId != SymbolId::Unknown;
}

} // namespace

Orderbook::Orderbook()
    : orderPool_(kOrderPoolChunkSize)
{
    orderPool_.preallocate(kPreallocatedOrderCapacity);
    orderLocators_.resize(kPreallocatedOrderCapacity + 1);
    overflowOrderLocators_.reserve(4096);
}

bool Orderbook::isKnownSymbol(SymbolId symbolId)
{
    return symbolId != SymbolId::Unknown;
}

Orderbook::SymbolBook& Orderbook::symbolBook(SymbolId symbolId)
{
    if (!isKnownSymbol(symbolId)) {
        throw std::out_of_range("Unknown symbol");
    }
    return books_[static_cast<std::size_t>(symbolId)];
}

const Orderbook::SymbolBook& Orderbook::symbolBook(SymbolId symbolId) const
{
    if (!isKnownSymbol(symbolId)) {
        throw std::out_of_range("Unknown symbol");
    }
    return books_[static_cast<std::size_t>(symbolId)];
}

bool Orderbook::hasOrderLocatorUnlocked(OrderId orderId) const
{
    if (orderId < orderLocators_.size()) {
        return orderLocators_[orderId].book_ != nullptr;
    }
    return overflowOrderLocators_.find(orderId) != overflowOrderLocators_.end();
}

Orderbook::OrderLocator* Orderbook::getOrderLocatorUnlocked(OrderId orderId)
{
    if (orderId < orderLocators_.size()) {
        auto& locator = orderLocators_[orderId];
        if (locator.book_ == nullptr) {
            return nullptr;
        }
        return &locator;
    }

    auto it = overflowOrderLocators_.find(orderId);
    if (it == overflowOrderLocators_.end()) {
        return nullptr;
    }
    return &it->second;
}

const Orderbook::OrderLocator* Orderbook::getOrderLocatorUnlocked(OrderId orderId) const
{
    if (orderId < orderLocators_.size()) {
        const auto& locator = orderLocators_[orderId];
        if (locator.book_ == nullptr) {
            return nullptr;
        }
        return &locator;
    }

    auto it = overflowOrderLocators_.find(orderId);
    if (it == overflowOrderLocators_.end()) {
        return nullptr;
    }
    return &it->second;
}

void Orderbook::upsertOrderLocatorUnlocked(OrderId orderId, OrderLocator locator)
{
    if (orderId < orderLocators_.size()) {
        orderLocators_[orderId] = std::move(locator);
        return;
    }

    overflowOrderLocators_.insert_or_assign(orderId, std::move(locator));
}

void Orderbook::eraseOrderLocatorUnlocked(OrderId orderId)
{
    if (orderId < orderLocators_.size()) {
        orderLocators_[orderId] = OrderLocator{};
        return;
    }
    overflowOrderLocators_.erase(orderId);
}

OrderPointer Orderbook::makePooledOrder(OrderId orderId, Price price, Quantity quantity, Side side, SymbolId symbolId)
{
    Order* raw = orderPool_.allocate(orderId, price, quantity, side, symbolId);
    return OrderPointer(raw, [this](Order* ptr) {
        orderPool_.deallocate(ptr);
    });
}

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

        SymbolId symbolId = SymbolId::Unknown;
        if (!parseSymbolId(fields.symbol, symbolId)) {
            return kErrResponse;
        }
        if (qty <= 0) {
            return kErrResponse;
        }
        if (price <= 0) {
            return kErrResponse;
        }

        modifyOrder(OrderModify{orderId, price, qty, side, symbolId});
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

    SymbolId symbolId = SymbolId::Unknown;
    if (!parseSymbolId(fields.symbol, symbolId)) {
        return kErrResponse;
    }
    if (qty <= 0) {
        return kErrResponse;
    }
    if (price <= 0) {
        return kErrResponse;
    }

    {
        std::scoped_lock lock(ordersMutex_);
        if (hasOrderLocatorUnlocked(orderId)) {
            return kErrResponse;
        }
    }

    // Step 3: Create and add the order
    auto order = makePooledOrder(orderId, price, qty, side, symbolId);
    
    addOrder(order);
    return std::string(kCreatedPrefix) + std::to_string(orderId);
}



Trades Orderbook::addOrder(const OrderPointer& order)
{
    if (!order) {
        return { };
    }

    if (!isKnownSymbol(order->getSymbolId())) {
        return { };
    }

    std::scoped_lock lock(ordersMutex_);

    if (hasOrderLocatorUnlocked(order->getOrderId())) {
        return { };
    }

    auto& book = symbolBook(order->getSymbolId());
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

    upsertOrderLocatorUnlocked(order->getOrderId(), OrderLocator{order, iterator, &book});

    return matchOrders(book);
}

void Orderbook::cancelOrder(OrderId orderId)
{
    std::scoped_lock lock(ordersMutex_);

    OrderLocator* locator = getOrderLocatorUnlocked(orderId);
    if (locator == nullptr || locator->book_ == nullptr) {
        return;
    }

    auto& book = *locator->book_;
    const auto order = locator->order_;
    const auto iterator = locator->location_;
    eraseOrderLocatorUnlocked(orderId);

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
    const OrderLocator* locator = nullptr;
    SymbolId existingSymbolId = SymbolId::Unknown;
    {
        std::scoped_lock lock(ordersMutex_);
        locator = getOrderLocatorUnlocked(order.getOrderId());
        if (locator == nullptr || locator->order_ == nullptr) {
            return { };
        }
        existingSymbolId = locator->order_->getSymbolId();
    }

    // Keep modification symbol-scoped to avoid moving an order across books implicitly.
    if (existingSymbolId != order.getSymbolId()) {
        return { };
    }

    cancelOrder(order.getOrderId());
    return addOrder(makePooledOrder(
        order.getOrderId(),
        order.getPrice(),
        order.getQuantity(),
        order.getSide(),
        order.getSymbolId()));
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
            TradeInfo{bestBidPrice, tradeQty, bidOrder->getOrderId(), Symbol(toSymbolString(bidOrder->getSymbolId()))},
            TradeInfo{bestAskPrice, tradeQty, askOrder->getOrderId(), Symbol(toSymbolString(askOrder->getSymbolId()))}
        });

        if (bidOrder->isFilled()) {
            bidQueue.pop_front();
            eraseOrderLocatorUnlocked(bidOrder->getOrderId());
        }

        if (askOrder->isFilled()) {
            askQueue.pop_front();
            eraseOrderLocatorUnlocked(askOrder->getOrderId());
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

    for (std::size_t i = 0; i < kKnownSymbolCount; ++i) {
        const SymbolId symbolId = static_cast<SymbolId>(i);
        const auto& book = books_[i];
        std::cout << "Symbol: " << toSymbolString(symbolId) << "\n";
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
