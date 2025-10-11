#include "Server.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

Server::Server(int port, Orderbook* orderbook)
    : port_(port), orderbook_(orderbook) {}

void Server::run() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Socket creation failed\n";
        return;
    }

    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed\n";
        close(server_fd);
        return;
    }

    if (listen(server_fd, 5) < 0) {
        std::cerr << "Listen failed\n";
        close(server_fd);
        return;
    }

    std::cout << "Server listening on port " << port_ << std::endl;

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            std::cerr << "Accept failed\n";
            continue;
        }
        handleClient(client_fd);
        close(client_fd);
    }

    close(server_fd);
}

void Server::handleClient(int clientSocket) {
    std::string jsonMsg = receiveMessage(clientSocket);
    if (jsonMsg.empty()) return;

    try {
        json request = json::parse(jsonMsg);
        json response = orderbook_->processJsonMessage(request);
        std::cout << "Processed message: " << response << std::endl;
        sendMessage(clientSocket, response.dump());
    } catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        sendMessage(clientSocket, R"({"error":"Invalid JSON"})");
    }
}

std::string Server::receiveMessage(int clientSocket) {
    char buffer[4096];
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) return "";
    buffer[bytesRead] = '\0';
    return std::string(buffer);
}

void Server::sendMessage(int clientSocket, const std::string& message) {
    send(clientSocket, message.c_str(), message.size(), 0);
}