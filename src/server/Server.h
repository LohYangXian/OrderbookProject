#pragma once

#include "Orderbook.h"
#include <string_view>
#include <string>

class Server {
public:
    Server(int port, Orderbook* orderbook);
    void run(); // Starts the server loop

private:
    int port_;
    Orderbook* orderbook_;
    void handleClient(int clientSocket);
    void sendMessage(int clientSocket, std::string_view message);
};