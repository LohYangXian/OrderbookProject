#include "Orderbook.h"
#include <cassert>
#include <iostream>

int main() {
    Orderbook ob;

    // 1. Add a new BUY order
    auto buyOrder = std::make_shared<Order>(1, 10000, 5, Side::BUY);
    ob.addOrder(buyOrder);
    assert(ob.getBids().size() == 1);
    assert(ob.getAsks().empty());

    // 2. Add a new SELL order that matches
    auto sellOrder = std::make_shared<Order>(2, 10000, 5, Side::SELL);
    auto trades = ob.addOrder(sellOrder);
    assert(trades.size() == 1); // Should match
    assert(ob.getBids().empty());
    assert(ob.getAsks().empty());

    // 3. Add a new BUY order, then modify it to cross the book
    auto buyOrder2 = std::make_shared<Order>(3, 9900, 10, Side::BUY);
    ob.addOrder(buyOrder2);
    assert(ob.getBids().size() == 1);

    OrderModify mod{3, 10100, 10, Side::BUY};
    trades = ob.modifyOrder(mod);
    assert(trades.empty()); // No asks to match

    // 4. Add a SELL order at 10100, should match with modified BUY
    auto sellOrder2 = std::make_shared<Order>(4, 10100, 10, Side::SELL);
    trades = ob.addOrder(sellOrder2);
    assert(trades.size() == 1);

    // 5. Add and then cancel an order
    auto buyOrder3 = std::make_shared<Order>(5, 9500, 5, Side::BUY);
    ob.addOrder(buyOrder3);
    ob.cancelOrder(5);
    assert(ob.getBids().empty());

    std::cout << "All tests passed!\n";
    return 0;
}