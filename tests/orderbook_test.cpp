#include "Orderbook.h"
#include <cassert>
#include <iostream>

int main() {
    Orderbook ob;
    const SymbolId symbol = SymbolId::NVDA;
    const SymbolId otherSymbol = SymbolId::AAPL;

    // 1. Add a new BUY order
    auto buyOrder = std::make_shared<Order>(1, 10000, 5, Side::BUY, symbol);
    ob.addOrder(buyOrder);
    assert(ob.getBids(symbol).size() == 1);
    assert(ob.getAsks(symbol).empty());

    // 1b. Add an order in another symbol and verify it does not match/collide.
    auto otherSell = std::make_shared<Order>(100, 10000, 5, Side::SELL, otherSymbol);
    auto otherTrades = ob.addOrder(otherSell);
    assert(otherTrades.empty());
    assert(ob.getAsks(otherSymbol).size() == 1);

    // 2. Add a new SELL order that matches
    auto sellOrder = std::make_shared<Order>(2, 10000, 5, Side::SELL, symbol);
    auto trades = ob.addOrder(sellOrder);
    assert(trades.size() == 1); // Should match
    assert(ob.getBids(symbol).empty());
    assert(ob.getAsks(symbol).empty());
    assert(ob.getAsks(otherSymbol).size() == 1); // Other symbol book untouched

    // 3. Add a new BUY order, then modify it to cross the book
    auto buyOrder2 = std::make_shared<Order>(3, 9900, 10, Side::BUY, symbol);
    ob.addOrder(buyOrder2);
    assert(ob.getBids(symbol).size() == 1);

    OrderModify mod{3, 10100, 10, Side::BUY, symbol};
    trades = ob.modifyOrder(mod);
    assert(trades.empty()); // No asks to match

    // 4. Add a SELL order at 10100, should match with modified BUY
    auto sellOrder2 = std::make_shared<Order>(4, 10100, 10, Side::SELL, symbol);
    trades = ob.addOrder(sellOrder2);
    assert(trades.size() == 1);

    // 5. Add and then cancel an order
    auto buyOrder3 = std::make_shared<Order>(5, 9500, 5, Side::BUY, symbol);
    ob.addOrder(buyOrder3);
    ob.cancelOrder(5);
    assert(ob.getBids(symbol).empty());

    ob.cancelOrder(100);
    assert(ob.getAsks(otherSymbol).empty());

    std::cout << "All tests passed!\n";
    return 0;
}