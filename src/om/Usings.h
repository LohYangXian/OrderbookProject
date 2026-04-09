#pragma once

#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <string_view>

using json = nlohmann::json;
using Price = uint32_t; // Price in cents to avoid floating point issues
using Quantity = uint32_t; // Quantity in smallest units (e.g., 0.001 BTC)
using OrderId = uint64_t; // Unique order identifier
using OrderIds = std::vector<OrderId>;
using Symbol = std::string; 

enum class SymbolId : uint8_t {
	NVDA,
	AAPL,
	TSLA,
	Unknown,
};

inline constexpr SymbolId toSymbolId(std::string_view symbol) {
	if (symbol == "NVDA") return SymbolId::NVDA;
	if (symbol == "AAPL") return SymbolId::AAPL;
	if (symbol == "TSLA") return SymbolId::TSLA;
	return SymbolId::Unknown;
}

inline constexpr std::string_view toSymbolString(SymbolId symbolId) {
	switch (symbolId) {
		case SymbolId::NVDA:
			return "NVDA";
		case SymbolId::AAPL:
			return "AAPL";
		case SymbolId::TSLA:
			return "TSLA";
		case SymbolId::Unknown:
		default:
			return "";
	}
}