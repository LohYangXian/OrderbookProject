#pragma once

#include <charconv>
#include <cstdint>
#include <string_view>
#include <vector>

using Price = uint32_t; // Price in cents to avoid floating point issues
using Quantity = uint32_t; // Quantity in smallest units (e.g., 0.001 BTC)
using OrderId = uint64_t; // Unique order identifier
using OrderIds = std::vector<OrderId>;
using SymbolId = uint32_t; // Symbol ID from FIX tag 55
using Symbol = SymbolId;

inline constexpr SymbolId kKnownSymbolCount = 500;
inline constexpr SymbolId kInvalidSymbolId = kKnownSymbolCount;

inline bool isValidSymbolId(SymbolId symbolId) {
    return symbolId < kKnownSymbolCount;
}

inline SymbolId toSymbolId(std::string_view symbol) {
    SymbolId id = 0;
    const char* begin = symbol.data();
    const char* end = begin + symbol.size();
    const auto [ptr, ec] = std::from_chars(begin, end, id);
    if (ec != std::errc() || ptr != end) {
        return kInvalidSymbolId;
    }
    return isValidSymbolId(id) ? id : kInvalidSymbolId;
}