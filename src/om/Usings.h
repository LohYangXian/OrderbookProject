#pragma once

#include <nlohmann/json.hpp>
#include <vector>

using json = nlohmann::json;
using Price = uint32_t; // Price in cents to avoid floating point issues
using Quantity = uint32_t; // Quantity in smallest units (e.g., 0.001 BTC)
using OrderId = uint64_t; // Unique order identifier
using OrderIds = std::vector<OrderId>;