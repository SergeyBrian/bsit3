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
    CMD_Add,  // NEW
    CMD_Srv,  // NEW
    CMD_Count_
};

inline const wchar_t *commandText[CMD_Count_] = {
    L"exit",   L"os",     L"time",  L"uptime",     L"memory",
    L"drives", L"rights", L"owner", L"disconnect",
    L"add",  // NEW
    L"srv",  // NEW
};

class Cli {
public:
    Cli();
    Cli(const std::string &host, const std::string &port);
    ~Cli();

    void run();

private:
    // Instead of a single connector, we now store a list of connectors:
    std::vector<connector::Connector *> m_connectors;

    // Keep track of the currently "active" server index:
    size_t m_activeServer = 0;

    // Helper method to parse commands
    static CMD parse_command(const std::wstring &command, int *argc,
                             wchar_t ***argv);

    // Method that executes the appropriate logic for a given command
    ERR exec(CMD cmd, int argc, wchar_t **argv);

    // Replaces old "inputConnectionInfo" usage. Now used to add a server.
    void inputConnectionInfo();

    // We have a parse port function that returns 0 on invalid parse
    static u16 ParsePort(const std::string &port_str);

    // Methods for each command:
    ERR getOsInfo();
    ERR getTime();
    ERR getUptime();
    ERR getMemory();
    ERR getDrives();
    ERR getRights(const wchar_t *path);
    ERR getOwner(const wchar_t *path);

    // NEW:
    // Command 'add <host> <port>'
    ERR addServer(int argc, wchar_t **argv);

    // Command 'srv <id>'
    ERR setActiveServer(int argc, wchar_t **argv);
};

inline Cli *g_instance = nullptr;

}  // namespace cli

#endif
