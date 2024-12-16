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
    CMD_Disconnect,
    CMD_Count_
};

inline const wchar_t *commandText[CMD_Count_] = {
    L"exit",   L"os",     L"time",  L"uptime",    L"memory",
    L"drives", L"rights", L"owner", L"disconnect",
};

class Cli {
public:
    Cli();

    Cli(const std::string &host, const std::string &port);

    ~Cli();

    void run();

private:
    connector::Connector *m_connector;

    static CMD parse_command(const std::wstring &command, int *argc,
                             wchar_t ***argv);

    ERR exec(CMD cmd, int argc, wchar_t **argv);

    void inputConnectionInfo();

    static u16 ParsePort(const std::string &port_str);

    ERR getOsInfo();

    ERR getTime();

    ERR getUptime();

    ERR getMemory();

    ERR getDrives();

    ERR getRights(const wchar_t *path);

    ERR getOwner(const wchar_t *path);
};

inline Cli *g_instance = nullptr;
}  // namespace cli

#endif