#include "server.h"

// Include networking headers (e.g., <sys/socket.h>, <netinet/in.h>, <unistd.h>)

Server::Server(int port, Orderbook* orderbook)
    : port_(port), orderbook_(orderbook) {}

void Server::run() {
    // Set up TCP socket, bind, listen, accept clients
    // For each client, call handleClient(clientSocket)
}

void Server::handleClient(int clientSocket) {
    while (true) {
        std::string jsonMsg = receiveMessage(clientSocket);
        // Parse JSON, process with orderbook
        // auto response = orderbook_->processJsonMessage(jsonMsg);
        // sendMessage(clientSocket, response.dump());
    }
}

std::string Server::receiveMessage(int clientSocket) {
    // Read from socket, return message as string
}

void Server::sendMessage(int clientSocket, const std::string& message) {
    // Write message to socket
}