#pragma once

#include <vector>

#include "TradeInfo.h"

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

using Trades = std::vector<Trade>;