#include "om/Orderbook.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace {

std::vector<std::string> buildWorkload(std::size_t count) {
    std::vector<std::string> messages;
    messages.reserve(count);

    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> sideDist(0, 1);
    std::uniform_int_distribution<int> symbolDist(0, 2);
    std::uniform_int_distribution<int> priceDist(90'000, 110'000);
    std::uniform_int_distribution<int> qtyDist(1, 10);

    static constexpr const char* kSymbols[] = {"NVDA", "AAPL", "TSLA"};

    for (std::size_t i = 0; i < count; ++i) {
        const std::uint64_t orderId = static_cast<std::uint64_t>(i + 1);
        const char* symbol = kSymbols[symbolDist(rng)];
        const char* side = sideDist(rng) == 0 ? "1" : "2";
        const std::uint32_t price = static_cast<std::uint32_t>(priceDist(rng));
        const std::uint32_t qty = static_cast<std::uint32_t>(qtyDist(rng));

        messages.push_back(
            std::string("8=FIX.4.2|35=D|11=") + std::to_string(orderId) +
            "|55=" + std::string(symbol) +
            "|54=" + std::string(side) +
            "|44=" + std::to_string(price) +
            "|38=" + std::to_string(qty) + "|");
    }

    return messages;
}

} // namespace

int main(int argc, char** argv) {
    const int durationSec = (argc > 1) ? std::max(1, std::atoi(argv[1])) : 10;
    const std::size_t workloadSize = (argc > 2) ? static_cast<std::size_t>(std::max(1000, std::atoi(argv[2]))) : 2'000'000;

    std::cout << "Engine benchmark starting\n";
    std::cout << "Duration: " << durationSec << "s\n";
    std::cout << "Pre-generated messages: " << workloadSize << "\n";

    Orderbook orderbook;
    const auto messages = buildWorkload(workloadSize);

    std::size_t processed = 0;
    std::size_t idx = 0;

    const auto start = std::chrono::steady_clock::now();
    const auto deadline = start + std::chrono::seconds(durationSec);
    while (std::chrono::steady_clock::now() < deadline) {
        orderbook.processFixMessage(messages[idx]);
        ++processed;
        ++idx;
        if (idx == messages.size()) {
            idx = 0;
        }
    }
    const auto end = std::chrono::steady_clock::now();

    const std::chrono::duration<double> elapsed = end - start;
    const double throughput = elapsed.count() > 0.0
        ? static_cast<double>(processed) / elapsed.count()
        : 0.0;

    std::cout << "Processed: " << processed << " messages\n";
    std::cout << "Elapsed: " << elapsed.count() << "s\n";
    std::cout << "Throughput: " << throughput << " msgs/s\n";

    return 0;
}
