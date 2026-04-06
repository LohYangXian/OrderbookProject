#include "server/Server.h"
#include "om/Orderbook.h"

int main() {
    Orderbook orderbook;
    Server server(8000, &orderbook); // Use desired port
    server.run();
    return 0;
}