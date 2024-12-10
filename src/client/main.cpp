#include "cli/cli.hpp"

int main(int argc, char **argv) {
    auto cli = cli::Cli();
    cli.run();
    return 0;
}