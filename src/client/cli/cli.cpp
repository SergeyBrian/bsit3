#include "cli.hpp"
#include "../../common/logging.hpp"
#include "../../common/utils.hpp"

namespace cli {

    Cli::Cli() : Cli("", "") {}


    Cli::Cli(const std::string &host, const std::string &port) {
        g_instance = this;


        u16 p = ParsePort(port);
        m_connector = new connector::Connector(host, p);
    }

    void Cli::run() {
        CMD cmd;
        do {
            if (!m_connector->canConnect()) {
                inputConnectionInfo();
            }
            std::wstring command;
            std::cout << m_connector->getHostStr() << " > ";
            std::getline(std::wcin >> std::ws, command);
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
        } while (cmd != CMD_Exit);

        LOG("Bye :)");
    }

    Cli::~Cli() {
        g_instance = nullptr;
        delete m_connector;
    }

    std::vector<std::wstring> split(const std::wstring &s) {
        std::vector<std::wstring> args;
        std::wstring current;
        bool in_quotes = false;
        for (const auto &c: s) {
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

    CMD Cli::parse_command(const std::wstring &command, int *argc, wchar_t ***argv) {
        if (*argv != nullptr) {
            delete[] reinterpret_cast<char*>(*argv);
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
            return cmd;
        }

        *argc = static_cast<int>(args.size());

        size_t argv_size = (*argc + 1) * sizeof(wchar_t *);
        size_t strings_size = 0;
        for (const auto &arg : args) {
            strings_size += (arg.size() + 1) * sizeof(wchar_t);
        }

        size_t total_size = argv_size + strings_size;

        char *block = new char[total_size];
        *argv = reinterpret_cast<wchar_t **>(block);

        wchar_t *strings = reinterpret_cast<wchar_t *>(block + argv_size);

        for (int i = 0; i < *argc; ++i) {
            (*argv)[i] = strings;
            std::memcpy(strings, args[i].c_str(), (args[i].size() + 1) * sizeof(wchar_t));
            strings += args[i].size() + 1;
        }
        (*argv)[*argc] = nullptr;

        return cmd;
    }

    ERR Cli::exec(CMD cmd, int argc, wchar_t **argv) {
        ERR err = ERR_Ok;
        switch (cmd) {
            case CMD_GetOsInfo:
                return getOsInfo();
            case CMD_GetTime:
                return getTime();
            case CMD_GetUptime:
                return getUptime();
            case CMD_GetMemory:
                return getMemory();
            case CMD_GetDrives:
                return getDrives();
            case CMD_GetRights:
                return getRights(argv[1]);
            case CMD_GetOwner:
                return getOwner(argv[1]);
            default:
                break;
        }

        return err;
    }

    void Cli::inputConnectionInfo() {
        std::string host;
        std::string port_str;

        do {
            u16 port = 0;

            while (!port) {
                std::cout << "Host: ";
                std::cin >> host;
                std::cout << "Port: ";
                std::cin >> port_str;

                port = ParsePort(port_str);
                m_connector->setServer(host, port);
            }
        } while (!m_connector->checkConnection() && WARN("Can't Connect to %s:%s", host.c_str(), port_str.c_str()));
    }

    u16 Cli::ParsePort(const std::string &port_str) {
        try {
            int port_i = std::stoi(port_str);
            if (port_i > static_cast<int>(UINT16_MAX) || port_i < 0) {
                throw std::out_of_range("port number exceeds 65535");
            }
            return static_cast<u16>(port_i);
        } catch (std::invalid_argument const &ex) {
            WARN("Invalid port value: %s", ex.what());
        } catch (std::out_of_range const &ex) {
            WARN("Port value too high: %s", ex.what());
        }

        return 0;
    }

    ERR Cli::getOsInfo() {
        OSInfo res{};
        ERR err = m_connector->getOsInfo(&res);
        if (err != ERR_Ok) {
            return err;
        }

        std::cout << OSTypeName[res.type] << " " << res.version.major << "." << res.version.minor << std::endl;

        return err;
    }

    ERR Cli::getTime() {
        return ERR_Ok;
    }

    ERR Cli::getUptime() {
        u64 uptime;
        ERR err = m_connector->getUptime(&uptime);
        if (err != ERR_Ok) {
            return err;
        }

        std::cout << "Time since start: " << utils::format_milliseconds(uptime) << std::endl;
        return err;
    }

    ERR Cli::getMemory() {
        MemInfo mem{};

        ERR err = m_connector->getMemory(&mem);
        if (err != ERR_Ok) {
            return err;
        }

        std::cout << "Total memory: " << utils::format_bytes(mem.total_bytes) << std::endl;
        std::cout << "Free memory: " << utils::format_bytes(mem.free_bytes) << std::endl;

        return err;
    }

    ERR Cli::getDrives() {
        std::vector<DriveInfo> drives;
        ERR err = m_connector->getDrives(&drives);
        if (err != ERR_Ok) {
            return err;
        }

        std::cout << "Mounted drives:\n";
        for (const auto &[type, name, free_bytes]: drives) {
            std::cout << "\t" << name << " [" << DriveTypeName[type] << "] Free space: "
                      << utils::format_bytes(free_bytes)
                      << std::endl;
        }

        return err;
    }

    ERR Cli::getRights(const wchar_t *path) {
        AccessRightsInfo info{};
        ERR err = m_connector->getRights(&info, path);

        utils::print_access_rights(info);
        return err;
    }

    ERR Cli::getOwner(const wchar_t *path) {
        OwnerInfo info{};
        ERR err = m_connector->getOwner(&info, path);

        std::cout << info.ownerDomain << "\\" << info.ownerName << "\n";
        return err;
    }
}
