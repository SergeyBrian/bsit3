#ifndef CLI_HPP
#define CLI_HPP

#include <iomanip>
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
    CMD_Add,
    CMD_Srv,
    CMD_Count_
};

inline const wchar_t *commandText[CMD_Count_] = {
    L"exit",   L"os",    L"time",       L"uptime", L"memory", L"drives",
    L"rights", L"owner", L"disconnect", L"add",    L"srv",
};

inline const wchar_t *commandDescription[CMD_Count_] = {
    L"quit client",
    L"get server os info",
    L"get current time on server",
    L"get server uptime",
    L"get server RAM info",
    L"get drives mounted to server",
    L"<path> get access rights to file at <path>",
    L"<path> get owner of file at <path>",
    L"close connection to current server",
    L"<ip> <port> connect to server",
    L"<number> switch to server",
};

inline void PrintHelp() {
    std::cout << "Available commands:\n";

    constexpr u8 columnWidth = 20;

    for (u8 i = 0; i < CMD_Count_; i++) {
        std::wcout << std::left << std::setw(columnWidth) << commandText[i]
                   << L" " << commandDescription[i] << "\n";
    }
}
class Cli {
public:
    Cli();
    Cli(const std::string &host, const std::string &port);
    ~Cli();

    void run();

private:
    std::vector<connector::Connector *> m_connectors;

    size_t m_activeServer = 0;

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

    ERR addServer(int argc, wchar_t **argv);

    ERR setActiveServer(int argc, wchar_t **argv);
};

inline Cli *g_instance = nullptr;

}  // namespace cli

#endif
