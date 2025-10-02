#pragma once
#include "om/Orderbook.h"
#include <string>

class Server {
public:
    Server(int port, Orderbook* orderbook);
    void run(); // Starts the server loop

private:
    int port_;
    Orderbook* orderbook_;
    void handleClient(int clientSocket);
    std::string receiveMessage(int clientSocket);
    void sendMessage(int clientSocket, const std::string& message);
};