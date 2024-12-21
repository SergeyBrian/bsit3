#include "server/handlers.hpp"

int main(int argc, char **argv) {
    u16 port = 6969;
    if (argc == 2) {
        auto portStr = std::string(argv[1]);
        port = std::stoi(portStr);
    }
    INFO("Using port %d", port);
    server::tcp::Server srv(port);
    server::handlers::Init(&srv);
    srv.Start();

    return 0;
}
