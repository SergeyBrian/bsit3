#include "cli.hpp"

#include <windows.h>
#include <sddl.h>

#include "../../common/logging.hpp"
#include "../../common/utils.hpp"
#include "../../common/errors.hpp"

#pragma comment(lib, "AdvApi32.lib")

namespace cli {

Cli::Cli() : Cli("", "") {}

Cli::Cli(const std::string &host, const std::string &port) {
    std::locale::global(std::locale("en_US.UTF-8"));
    std::wcin.imbue(std::locale());
    std::wcout.imbue(std::locale());
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    g_instance = this;

    if (!host.empty() && !port.empty()) {
        u16 p = ParsePort(port);
        if (p != 0) {
            auto *conn = new connector::Connector(m_connectors.size() + 1, host, p);
            if (conn->checkConnection()) {
                m_connectors.push_back(conn);
                m_activeServer = 0;
            } else {
                WARN("Could not connect to provided server %s:%s", host.c_str(),
                     port.c_str());
                delete conn;
            }
        }
    }
}

Cli::~Cli() {
    g_instance = nullptr;
    for (auto *c : m_connectors) {
        if (c) {
            c->disconnect();
            delete c;
        }
    }
    m_connectors.clear();
}

void Cli::run() {
    CMD cmd;
    do {
        if (m_connectors.empty() ||
            !m_connectors[m_activeServer]->canConnect()) {
            inputConnectionInfo();
            if (m_connectors.empty()) {
                std::cout
                    << "(No servers configured yet; use 'add <host> <port>')\n";
            }
        }

        std::wstring command;
        if (!m_connectors.empty()) {
            std::cout << "[" << m_activeServer << "] "
                      << m_connectors[m_activeServer]->getHostStr() << " > ";
        } else {
            std::cout << "[X] > ";
        }

        std::getline(std::wcin >> std::ws, command);
        INFO("%S", command.data());
        if (command.empty()) {
            cmd = CMD_Count_;
            continue;
        }

        int argc;
        wchar_t **argv = nullptr;
        cmd = parse_command(command, &argc, &argv);

        ERR err = exec(cmd, argc, argv);
        if (err) {
            WARN("%s", errorText[err]);
        }
        if (argv) {
            delete[] reinterpret_cast<char *>(argv);
            argv = nullptr;
        }

    } while (cmd != CMD_Exit);

    LOG("Bye :)");
}

std::vector<std::wstring> split(const std::wstring &s) {
    std::vector<std::wstring> args;
    std::wstring current;
    bool in_quotes = false;
    for (auto c : s) {
        if (c == L'"') {
            in_quotes = !in_quotes;
        } else if (c == L' ' && !in_quotes) {
            if (!current.empty()) {
                args.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        args.push_back(current);
    }
    return args;
}

CMD Cli::parse_command(const std::wstring &command, int *argc,
                       wchar_t ***argv) {
    if (*argv != nullptr) {
        delete[] reinterpret_cast<char *>(*argv);
        *argv = nullptr;
    }

    std::vector<std::wstring> args = split(command);

    CMD cmd = CMD_Count_;
    if (!args.empty()) {
        for (int i = 0; i < CMD_Count_; i++) {
            if (args[0] == commandText[i]) {
                cmd = static_cast<CMD>(i);
                break;
            }
        }
    }

    if (cmd == CMD_Count_) {
        if (!args.empty()) {
            WARN("Unknown command: %S", args[0].c_str());
        } else {
            WARN("Empty command");
        }
        *argc = 0;
        return cmd;
    }

    *argc = static_cast<int>(args.size());

    usize argv_size = (*argc + 1) * sizeof(wchar_t *);
    usize strings_size = 0;
    for (auto &arg : args) {
        strings_size += (arg.size() + 1) * sizeof(wchar_t);
    }
    usize total_size = argv_size + strings_size;

    char *block = new char[total_size];
    *argv = reinterpret_cast<wchar_t **>(block);

    wchar_t *strings = reinterpret_cast<wchar_t *>(block + argv_size);

    for (int i = 0; i < *argc; i++) {
        (*argv)[i] = strings;
        std::memcpy(strings, args[i].c_str(),
                    (args[i].size() + 1) * sizeof(wchar_t));
        strings += (args[i].size() + 1);
    }
    (*argv)[*argc] = nullptr;

    return cmd;
}

ERR Cli::exec(CMD cmd, int argc, wchar_t **argv) {
    auto activeConn = [&]() -> connector::Connector * {
        if (m_connectors.empty() || m_activeServer >= m_connectors.size()) {
            WARN("No active server is available.");
            return nullptr;
        }
        return m_connectors[m_activeServer];
    };

    switch (cmd) {
        case CMD_Exit:
            return ERR_Ok;

        case CMD_GetOsInfo:
            if (auto *c = activeConn()) {
                return getOsInfo();
            }
            return ERR_Connect;

        case CMD_GetTime:
            if (auto *c = activeConn()) {
                return getTime();
            }
            return ERR_Connect;

        case CMD_GetUptime:
            if (auto *c = activeConn()) {
                return getUptime();
            }
            return ERR_Connect;

        case CMD_GetMemory:
            if (auto *c = activeConn()) {
                return getMemory();
            }
            return ERR_Connect;

        case CMD_GetDrives:
            if (auto *c = activeConn()) {
                return getDrives();
            }
            return ERR_Connect;

        case CMD_GetRights:
            if (argc < 2) {
                WARN("Usage: rights <path>");
                return ERR_InvalidArgument;
            }
            if (auto *c = activeConn()) {
                return getRights(argv[1]);
            }
            return ERR_Connect;

        case CMD_GetOwner:
            if (argc < 2) {
                WARN("Usage: owner <path>");
                return ERR_InvalidArgument;
            }
            if (auto *c = activeConn()) {
                return getOwner(argv[1]);
            }
            return ERR_Connect;

        case CMD_Disconnect:
            if (auto *c = activeConn()) {
                c->disconnect();
            }
            return ERR_Ok;

        case CMD_Add:
            return addServer(argc, argv);

        case CMD_Srv:
            return setActiveServer(argc, argv);

        default:
            break;
    }
    return ERR_Ok;
}

void Cli::inputConnectionInfo() {
    std::string host;
    std::string port_str;
    u16 port = 0;

    while (port == 0) {
        std::cout << "Host: ";
        std::cin >> host;
        std::cout << "Port: ";
        std::cin >> port_str;

        port = ParsePort(port_str);
        if (port == 0) {
            WARN("Invalid port. Please try again.");
            continue;
        }

        auto *conn = new connector::Connector(m_connectors.size() + 1, host, port);
        if (!conn->checkConnection()) {
            WARN("Can't Connect to %s:%s", host.c_str(), port_str.c_str());
            delete conn;
            port = 0;  // re-trigger the loop
        } else {
            m_connectors.push_back(conn);
            // If this is the only server, set it as active
            if (m_connectors.size() == 1) {
                m_activeServer = 0;
            }
        }
    }
}

u16 Cli::ParsePort(const std::string &port_str) {
    try {
        int port_i = std::stoi(port_str);
        if (port_i > static_cast<int>(UINT16_MAX) || port_i < 0) {
            throw std::out_of_range("port number out of range");
        }
        return static_cast<u16>(port_i);
    } catch (std::invalid_argument const &ex) {
        WARN("Invalid port value: %s", ex.what());
    } catch (std::out_of_range const &ex) {
        WARN("Port value too high/low: %s", ex.what());
    }
    return 0;
}

ERR Cli::getOsInfo() {
    auto *conn = m_connectors[m_activeServer];
    OSInfo res{};
    ERR err = conn->getOsInfo(&res);
    if (err != ERR_Ok) {
        return err;
    }
    std::cout << OSTypeName[res.type] << " "
              << static_cast<u32>(res.version.major) << "."
              << static_cast<u32>(res.version.minor) << std::endl;
    return err;
}

ERR Cli::getTime() {
    auto *conn = m_connectors[m_activeServer];
    u64 time;
    i8 time_zone;
    ERR err = conn->getTime(&time, &time_zone);
    if (err != ERR_Ok) {
        return err;
    }
    std::cout << utils::format_time(time + time_zone * 3600 * 1000) << " (UTC "
              << ((time_zone < 0) ? "-" : "+") << abs(time_zone) << ")"
              << std::endl;
    return err;
}

ERR Cli::getUptime() {
    auto *conn = m_connectors[m_activeServer];
    u64 uptime;
    ERR err = conn->getUptime(&uptime);
    if (err != ERR_Ok) {
        return err;
    }
    std::cout << "Time since start: " << utils::format_milliseconds(uptime)
              << std::endl;
    return err;
}

ERR Cli::getMemory() {
    auto *conn = m_connectors[m_activeServer];
    MemInfo mem{};
    ERR err = conn->getMemory(&mem);
    if (err != ERR_Ok) {
        return err;
    }
    std::cout << "Total memory: " << utils::format_bytes(mem.total_bytes)
              << std::endl;
    std::cout << "Free memory: " << utils::format_bytes(mem.free_bytes)
              << std::endl;
    return err;
}

ERR Cli::getDrives() {
    auto *conn = m_connectors[m_activeServer];
    std::vector<DriveInfo> drives;
    ERR err = conn->getDrives(&drives);
    if (err != ERR_Ok) {
        return err;
    }
    std::cout << "Mounted drives:\n";
    for (const auto &[type, name, free_bytes] : drives) {
        std::cout << "\t" << name << " [" << DriveTypeName[type]
                  << "] Free space: " << utils::format_bytes(free_bytes)
                  << std::endl;
    }
    return err;
}

ERR Cli::getRights(const wchar_t *path) {
    auto *conn = m_connectors[m_activeServer];
    AccessRightsInfo info{};
    ERR err = conn->getRights(&info, path);
    if (err != ERR_Ok) {
        return err;
    }
    utils::print_access_rights(info);
    return err;
}

ERR Cli::getOwner(const wchar_t *path) {
    auto *conn = m_connectors[m_activeServer];
    OwnerInfo info{};
    ERR err = conn->getOwner(&info, path);
    if (err != ERR_Ok) {
        return err;
    }
    std::cout << info.ownerDomain << "\\" << info.ownerName << " ";
    char *sidString = nullptr;
    if (ConvertSidToStringSidA(
            reinterpret_cast<PSID>(
                const_cast<unsigned char *>(info.sid.data())),
            &sidString)) {
        std::cout << "SID: " << sidString << "\n";
        LocalFree(sidString);
    } else {
        std::cout << "SID: (failed to convert SID to string format)\n";
    }
    return err;
}

ERR Cli::addServer(int argc, wchar_t **argv) {
    if (argc < 3) {
        WARN("Usage: add <host> <port>");
        return ERR_InvalidArgument;
    }

    std::string host = utils::to_string(argv[1]);
    std::string port_str = utils::to_string(argv[2]);
    u16 port = ParsePort(port_str);

    if (port == 0) {
        return ERR_InvalidArgument;
    }

    auto *conn = new connector::Connector(m_connectors.size() + 1, host, port);
    if (!conn->checkConnection()) {
        WARN("Could not connect to: %s:%s", host.c_str(), port_str.c_str());
        delete conn;
        return ERR_Connect;
    }

    m_connectors.push_back(conn);
    m_activeServer = m_connectors.size() - 1;

    INFO("Server added. Active server is now index %zu", m_activeServer);
    return ERR_Ok;
}

ERR Cli::setActiveServer(int argc, wchar_t **argv) {
    // Usage: srv <id>
    if (argc < 2) {
        WARN("Usage: srv <id>");
        return ERR_InvalidArgument;
    }

    int id = 0;
    try {
        id = std::stoi(utils::to_string(argv[1]));
    } catch (...) {
        WARN("Invalid server ID format: %S", argv[1]);
        return ERR_InvalidArgument;
    }

    if (id < 0 || static_cast<size_t>(id) >= m_connectors.size()) {
        WARN("No server with index %d found.", id);
        return ERR_NotFound;
    }

    m_activeServer = static_cast<size_t>(id);
    INFO("Switched to server index %zu", m_activeServer);

    if (!m_connectors[m_activeServer]->canConnect()) {
        WARN("Server %d is not connected. Attempting reconnect...", id);
        if (!m_connectors[m_activeServer]->checkConnection()) {
            WARN("Reconnect attempt failed.");
            return ERR_Connect;
        }
    }

    return ERR_Ok;
}
}  // namespace cli
