#include "server/handlers.hpp"
#include "test.hpp"

int main(int argc, char **argv) {
#ifndef NDEBUG
    test::PrintTestOutput(argv);
#endif

    server::tcp::Server srv(6969);
    server::handlers::Init(&srv);
    srv.Start();

    return 0;
}
