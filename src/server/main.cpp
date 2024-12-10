#include "server/server.hpp"
#include "test.hpp"

int main(int argc, char **argv) {
#ifndef NDEBUG
    test::PrintTestOutput(argv);
#endif

    server::Server srv(6969);
    srv.run();


    return 0;
}
