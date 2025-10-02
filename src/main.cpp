#include <iostream>
#include <random>
#include <vector>
#include "om/Orderbook.h"
#include <chrono>
#include <vector>


// Helper to generate random orders
// TODO: Add random generation of different order types
Order generateRandomOrder() {
    static int orderId = 1;
    static std::default_random_engine rng(std::random_device{}());
    std::uniform_int_distribution<int> sideDist(0, 1);
    std::uniform_int_distribution<int> priceDist(9900000, 11000000);
    std::uniform_int_distribution<int> qtyDist(10, 10000);

    Order order(orderId++, priceDist(rng), qtyDist(rng), sideDist(rng) == 0 ? Side::BUY : Side::SELL);
    return order;
}

json generateRandomOrderJson() {
    static int orderId = 1;
    static std::default_random_engine rng(std::random_device{}());
    std::uniform_int_distribution<int> sideDist(0, 1);
    std::uniform_int_distribution<int> priceDist(9900, 11000);
    std::uniform_int_distribution<int> qtyDist(1, 100);

    json j;
    j["OrderId"] = orderId++;
    j["Pair"] = "BTC/USDT";
    j["Price"] = std::to_string(priceDist(rng));
    j["Quantity"] = std::to_string(qtyDist(rng));
    j["Side"] = sideDist(rng) == 0 ? "BUY" : "SELL";
    return j;
}

int main() {
    Orderbook orderbook;
    std::string line;

    std::vector<long long> latencies_ns; // nanoseconds

    const int NUM_ORDERS = 1000000;
    for (int i = 0; i < NUM_ORDERS; ++i) {
        // Order randomOrder = generateRandomOrder();
        json randomOrder = generateRandomOrderJson();
        // std::cout << "Injecting random order: "
                // << (randomOrder["Side"] == "BUY" ? "Buy" : "Sell")
                // << " " << randomOrder["Quantity"] << " @ " << randomOrder["Price"] << "\n";
        auto start = std::chrono::high_resolution_clock::now();
        auto response = orderbook.processJsonMessage(randomOrder);
        // orderbook.addOrder(std::make_shared<Order>(randomOrder));
        auto end = std::chrono::high_resolution_clock::now();
        auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        latencies_ns.push_back(latency);
        std::cout << "Response: " << response.dump() << "\n";
    }

    // Compute average latency
    // TODO: Compute average latency for different order types
    if (!latencies_ns.empty()) {
        long long sum = 0;
        for (auto ns : latencies_ns) sum += ns;
        double avg_ns = static_cast<double>(sum) / latencies_ns.size();
        std::cout << "Average order matching latency: " << avg_ns << " ns (" << avg_ns / 1e6 << " ms)\n";
    }

    return 0;
}