#include <iostream>
#include <random>
#include <vector>
#include "om/Orderbook.h"
#include <chrono>
#include <vector>

std::string generateRandomFixMessage() {
    static int orderId = 1;
    static std::default_random_engine rng(std::random_device{}());
    std::uniform_int_distribution<int> typeDist(0, 2); // 0: NEW, 1: MODIFY, 2: CANCEL
    std::uniform_int_distribution<int> sideDist(0, 1);
    std::uniform_int_distribution<int> priceDist(9900, 11000);
    std::uniform_int_distribution<int> qtyDist(1, 100);

    int type = typeDist(rng);
    constexpr const char* symbol = "NVDA";

    if (type == 1 && orderId > 1) { // MODIFY existing order
        std::uniform_int_distribution<int> existingOrderIdDist(1, orderId - 1);
        return "8=FIX.4.2|35=G|11=" + std::to_string(existingOrderIdDist(rng))
            + "|55=" + symbol
            + "|54=" + std::to_string(sideDist(rng) == 0 ? 1 : 2)
            + "|44=" + std::to_string(priceDist(rng))
            + "|38=" + std::to_string(qtyDist(rng)) + "|";
    } else if (type == 2 && orderId > 1) { // CANCEL existing order
        std::uniform_int_distribution<int> existingOrderIdDist(1, orderId - 1);
        return "8=FIX.4.2|35=F|11=" + std::to_string(existingOrderIdDist(rng)) + "|";
    } else {
        // Fallback to NEW if no existing orders to modify/cancel
        const int currentOrderId = orderId++;
        return "8=FIX.4.2|35=D|11=" + std::to_string(currentOrderId)
            + "|55=" + symbol
            + "|54=" + std::to_string(sideDist(rng) == 0 ? 1 : 2)
            + "|44=" + std::to_string(priceDist(rng))
            + "|38=" + std::to_string(qtyDist(rng)) + "|";
    }
}

int main() {
    Orderbook orderbook;
    std::string line;

    std::vector<long long> latencies_ns; // nanoseconds

    const int NUM_ORDERS = 1000000;
    for (int i = 0; i < NUM_ORDERS; ++i) {
        std::string fixMessage = generateRandomFixMessage();
        auto start = std::chrono::high_resolution_clock::now();
        std::string response = orderbook.processFixMessage(fixMessage);
        auto end = std::chrono::high_resolution_clock::now();
        auto latency = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        latencies_ns.push_back(latency);
        std::cout << "Response: " << response << "\n";
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