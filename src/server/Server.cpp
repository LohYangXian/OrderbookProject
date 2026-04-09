#include "Server.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <thread>
#include <string_view>

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

    if (listen(server_fd, 128) < 0) {
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

        // Low-latency responses for small FIX messages.
        int nodelay = 1;
        if (setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay)) < 0) {
            std::cerr << "setsockopt(TCP_NODELAY) failed\n";
            close(client_fd);
            continue;
        }

        std::thread(&Server::handleClient, this, client_fd).detach();
    }

    close(server_fd);
}

void Server::handleClient(int clientSocket) {
    constexpr std::size_t kReadBufferSize = 64 * 1024;
    constexpr std::size_t kMaxFrameBytes = 4 * 1024;

    char readBuffer[kReadBufferSize];
    std::string receiveBuffer;
    std::string sendBuffer;
    receiveBuffer.reserve(kReadBufferSize);
    sendBuffer.reserve(kReadBufferSize);

    while (true) {
        const ssize_t bytesRead = recv(clientSocket, readBuffer, sizeof(readBuffer), 0);
        if (bytesRead <= 0) {
            close(clientSocket);
            return;
        }

        receiveBuffer.append(readBuffer, static_cast<std::size_t>(bytesRead));

        std::size_t frameEnd = receiveBuffer.find('\n');
        while (frameEnd != std::string::npos) {
            std::string_view frame(receiveBuffer.data(), frameEnd);
            if (!frame.empty() && frame.back() == '\r') {
                frame.remove_suffix(1);
            }

            if (!frame.empty()) {
                const std::string response = orderbook_->processFixMessage(std::string(frame));
                sendBuffer.append(response);
                sendBuffer.push_back('\n');
            }

            receiveBuffer.erase(0, frameEnd + 1);
            frameEnd = receiveBuffer.find('\n');
        }

        if (!sendBuffer.empty()) {
            sendMessage(clientSocket, sendBuffer);
            sendBuffer.clear();
        }

        if (receiveBuffer.size() > kMaxFrameBytes) {
            close(clientSocket);
            return;
        }
    }
}

void Server::sendMessage(int clientSocket, std::string_view message) {
    const char* data = message.data();
    size_t totalSent = 0;
    while (totalSent < message.size()) {
        const ssize_t sent = send(clientSocket,
                                  data + totalSent,
                                  message.size() - totalSent,
                                  MSG_NOSIGNAL);
        if (sent <= 0) {
            return;
        }
        totalSent += static_cast<size_t>(sent);
    }
}