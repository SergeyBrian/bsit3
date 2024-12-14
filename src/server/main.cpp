#include "server/handlers.hpp"

int main(int argc, char **argv) {
    server::tcp::Server srv(6969);
    server::handlers::Init(&srv);
    srv.Start();

    return 0;
}
