#ifndef CLI_HPP
#define CLI_HPP

#include <string>
#include <vector>
#include "../../common/alias.hpp"
#include "../connector/connector.hpp"
#include "../../common/errors.hpp"

namespace cli {
    enum CMD : u8 {
        CMD_Nop = static_cast<u8>(~0),
        CMD_Exit = 0,
        CMD_Count_
    };

    inline const char *commandText[CMD_Count_] = {
            "exit",
    };

    class Cli {
    public:
        Cli();

        Cli(const std::string &host, const std::string &port);

        ~Cli();

        void run();

    private:
        connector::Connector *m_connector;

        CMD parse_command(const std::string &command, int *argc, char ***argv);

        ERR exec(CMD cmd, int argc, char **argv);

        void inputConnectionInfo();

        static u16 ParsePort(const std::string &port_str);
    };

    inline Cli *g_instance = nullptr;
}

#endif