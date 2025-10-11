#include <iostream>
#include <random>
#include <vector>
#include "om/Orderbook.h"
#include <chrono>
#include <vector>

json generateRandomOrderJson() {
    static int orderId = 1;
    static std::default_random_engine rng(std::random_device{}());
    std::uniform_int_distribution<int> typeDist(0, 2); // 0: NEW, 1: MODIFY, 2: CANCEL
    std::uniform_int_distribution<int> sideDist(0, 1);
    std::uniform_int_distribution<int> priceDist(9900, 11000);
    std::uniform_int_distribution<int> qtyDist(1, 100);

    json j;
    int type = typeDist(rng);
    // NEW Order
    if (type == 1 && orderId > 1) { // MODIFY existing order
        std::uniform_int_distribution<int> existingOrderIdDist(1, orderId - 1);
        j["Type"] = "MODIFY";
        j["OrderId"] = existingOrderIdDist(rng);
        j["Pair"] = "BTC/USDT";
        j["Price"] = std::to_string(priceDist(rng));
        j["Quantity"] = std::to_string(qtyDist(rng));
        j["Side"] = sideDist(rng) == 0 ? "BUY" : "SELL";
    } else if (type == 2 && orderId > 1) { // CANCEL existing order
        std::uniform_int_distribution<int> existingOrderIdDist(1, orderId - 1);
        j["Type"] = "CANCEL";
        j["OrderId"] = existingOrderIdDist(rng);
    } else {
        // Fallback to NEW if no existing orders to modify/cancel
        j["Type"] = "NEW";
        j["OrderId"] = orderId++;
        j["Pair"] = "BTC/USDT";
        j["Price"] = std::to_string(priceDist(rng));
        j["Quantity"] = std::to_string(qtyDist(rng));
        j["Side"] = sideDist(rng) == 0 ? "BUY" : "SELL";
    }
    return j;
}

int main() {
    Orderbook orderbook;
    std::string line;

    std::vector<long long> latencies_ns; // nanoseconds

    const int NUM_ORDERS = 1000000;
    for (int i = 0; i < NUM_ORDERS; ++i) {
        json randomOrder = generateRandomOrderJson();
        auto start = std::chrono::high_resolution_clock::now();
        auto response = orderbook.processJsonMessage(randomOrder);
        auto end = std::chrono::high_resolution_clock::now();
        auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        latencies_ns.push_back(latency);
        std::cout << "Response: " << response.dump() << "\n";
    }

    // Compute average latency
    if (!latencies_ns.empty()) {
        long long sum = 0;
        for (auto ns : latencies_ns) sum += ns;
        double avg_ns = static_cast<double>(sum) / latencies_ns.size();
        std::cout << "Average order matching latency: " << avg_ns << " ns (" << avg_ns / 1e6 << " ms)\n";
    }

    return 0;
}