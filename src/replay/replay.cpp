#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>

// Type alias for WebSocket++ client
typedef websocketpp::client<websocketpp::config::asio_client> client;

// Message handler: print incoming JSON payloads
void on_message(websocketpp::connection_hdl, client::message_ptr msg) {
    std::cout << msg->get_payload() << std::endl;
}

int main() {
    client c;
    c.init_asio();
    c.set_message_handler(&on_message);

    // Binance futures depth stream for BTCUSDT
    std::string uri = "wss://fstream.binance.com:9443/ws/btcusdt@depth";

    websocketpp::lib::error_code ec;
    client::connection_ptr con = c.get_connection(uri, ec);
    if (ec) {
        std::cerr << "Connection failed: " << ec.message() << std::endl;
        return 1;
    }

    c.connect(con);
    c.run();
    return 0;
}