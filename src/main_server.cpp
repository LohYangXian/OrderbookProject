#include "server/Server.h"
#include "om/Orderbook.h"

int main() {
    signal(SIGINT, handle_sigint);
    Orderbook orderbook;
    Server server(8000, &orderbook); // Use desired port
    server.run();
    return 0;
}