#include "server/Server.h"
#include "om/Orderbook.h"

#include <gperftools/profiler.h>

// Handle SIGINT to stop profiling gracefully and flush data
void handle_sigint(int) {
    ProfilerStop();
    exit(0);
}

int main() {
    signal(SIGINT, handle_sigint);
    ProfilerStart("/tmp/orderbook_server.prof");
    Orderbook orderbook;
    Server server(8000, &orderbook); // Use desired port
    server.run();
    ProfilerStop();
    return 0;
}