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
        CMD_GetOsInfo,
        CMD_GetTime,
        CMD_GetUptime,
        CMD_GetMemory,
        CMD_GetDrives,
        CMD_GetRights,
        CMD_GetOwner,
        CMD_Count_
    };

    inline const char *commandText[CMD_Count_] = {
            "exit",
            "os",
            "time",
            "uptime",
            "memory",
            "drives",
            "rights",
            "owner",
    };

    class Cli {
    public:
        Cli();

        Cli(const std::string &host, const std::string &port);

        ~Cli();

        void run();

    private:
        connector::Connector *m_connector;

        static CMD parse_command(const std::string &command, int *argc, char ***argv);

        ERR exec(CMD cmd, int argc, char **argv);

        void inputConnectionInfo();

        static u16 ParsePort(const std::string &port_str);

        ERR getOsInfo();

        ERR getTime();

        ERR getUptime();

        ERR getMemory();

        ERR getDrives();

        ERR getRights(const char *path);

        ERR getOwner(const char *path);
    };

    inline Cli *g_instance = nullptr;
}

#endif