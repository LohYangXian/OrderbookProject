#include "Server.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <sstream>
#include <unordered_map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Detect whether a raw message is a FIX-style pipe-delimited string
// (e.g. ts=1000|client=1|cmd=N|id=1001|sym=AAPL|side=B|px=15025|qty=200)
// rather than a JSON object (which always starts with '{').
static bool isFixMessage(const std::string& msg) {
    return !msg.empty() && msg[0] != '{';
}

// Convert a FIX-style pipe-delimited message into the JSON object expected by
// Orderbook::processJsonMessage().  Field mapping:
//   cmd  : N -> NEW, M -> MODIFY, C -> CANCEL  (-> "Type")
//   id   : order id                             (-> "OrderId", numeric)
//   sym  : instrument symbol                    (-> "Pair")
//   side : B -> BUY, S -> SELL                  (-> "Side")
//   px   : price                                (-> "Price", string)
//   qty  : quantity                             (-> "Quantity", string)
//   ts, client are accepted but not forwarded.
static json fixToJson(const std::string& msg) {
    std::unordered_map<std::string, std::string> fields;
    std::istringstream ss(msg);
    std::string token;
    while (std::getline(ss, token, '|')) {
        auto pos = token.find('=');
        if (pos != std::string::npos)
            fields[token.substr(0, pos)] = token.substr(pos + 1);
    }

    json result;
    const std::string& cmd = fields["cmd"];
    if      (cmd == "N") result["Type"] = "NEW";
    else if (cmd == "C") result["Type"] = "CANCEL";
    else if (cmd == "M") result["Type"] = "MODIFY";

    if (fields.count("id"))   result["OrderId"]  = std::stoull(fields.at("id"));
    if (fields.count("sym"))  result["Pair"]      = fields.at("sym");
    if (fields.count("side")) result["Side"]      = (fields.at("side") == "B") ? "BUY" : "SELL";
    if (fields.count("px"))   result["Price"]     = fields.at("px");
    if (fields.count("qty"))  result["Quantity"]  = fields.at("qty");
    return result;
}

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
    std::string msg = receiveMessage(clientSocket);
    if (msg.empty()) return;

    try {
        json request = isFixMessage(msg) ? fixToJson(msg) : json::parse(msg);
        json response = orderbook_->processJsonMessage(request);
        std::cout << "Processed message: " << response << std::endl;
        sendMessage(clientSocket, response.dump());
    } catch (const std::exception& e) {
        std::cerr << "Parse error: " << e.what() << std::endl;
        sendMessage(clientSocket, R"({"error":"Invalid message"})");
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