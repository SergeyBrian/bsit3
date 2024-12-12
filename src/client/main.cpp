#include "cli/cli.hpp"

int main(int argc, char **argv) {
    std::string host;
    std::string port;
    if (argc == 3) {
        host.assign(argv[1]);
        port.assign(argv[2]);
    }
    auto cli = cli::Cli(host, port);
    cli.run();
    return 0;
}