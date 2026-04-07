#include "Server.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

Server::Server(int port, Orderbook* orderbook)
    : port_(port), orderbook_(orderbook) {}

void Server::run() {
    // Notes:
    // AF_INET: IPv4
    // SOCK_STREAM: TCP
    // 0: default protocol (TCP for SOCK_STREAM)
    // UDP would be SOCK_DGRAM
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Socket creation failed\n";
        return;
    }

    // Allow quick server restarts without waiting for old socket state to clear.
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt(SO_REUSEADDR) failed\n";
        close(server_fd);
        return;
    }
#ifdef SO_REUSEPORT
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt(SO_REUSEPORT) failed\n";
        close(server_fd);
        return;
    }
#endif

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
    while (true) {
        std::string fixMessage = receiveMessage(clientSocket);
        if (fixMessage.empty()) {
            return;
        }

        const std::string response = orderbook_->processFixMessage(fixMessage);
        sendMessage(clientSocket, response);
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