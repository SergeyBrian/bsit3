#ifndef CLI_HPP
#define CLI_HPP

#include <string>
#include <vector>
#include "../../common/alias.hpp"
#include "../connector/connector.hpp"
#include "../../common/errors.hpp"

namespace cli {
    enum CMD : u8 {
        CMD_Exit,
        CMD_Count_
    };

    inline const char *commandText[CMD_Count_] = {
            "exit",
    };

    class Cli {
    public:
        Cli();

        Cli(const std::string &host, u16 port);

        ~Cli();

        void run();

    private:
        connector::Connector *m_connector;

        CMD parse_command(const std::string& command, int *argc, char ***argv);

        ERR exec(CMD cmd, int argc, char **argv);

        void inputConnectionInfo();
    };

    inline Cli *g_instance = nullptr;
}

#endif