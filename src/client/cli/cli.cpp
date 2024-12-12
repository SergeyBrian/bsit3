#include "cli.hpp"
#include "../../common/logging.hpp"

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
            std::string command;
            std::cout << m_connector->getHostStr() << " > ";
            std::getline(std::cin >> std::ws, command);
            if (command.empty()) {
                cmd = CMD_Nop;
                continue;
            }

            int argc;
            char **argv = nullptr;

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

    std::vector<std::string> split(const std::string &s) {
        std::vector<std::string> args;
        std::string current;
        bool in_quotes = false;
        for (char c: s) {
            if (c == '"') {
                in_quotes = !in_quotes;
            } else if (c == ' ' && !in_quotes) {
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

    CMD Cli::parse_command(const std::string &command, int *argc, char ***argv) {
        // Delete argv created by previous parse
        if (*argv != nullptr) {
            delete *argv;
        }

        std::vector<std::string> args = split(command);

        CMD cmd = CMD_Count_;
        for (int i = 0; i < CMD_Count_; i++) {
            if (args[0] == commandText[i]) {
                cmd = static_cast<CMD>(i);
                break;
            }
        }

        if (cmd == CMD_Count_) {
            WARN("Unknown command: %s", args[0].c_str());
            return cmd;
        }

        *argc = static_cast<int>(args.size());

        size_t argv_size = (args.size() + 1) * sizeof(char *);
        size_t strings_size = 0;
        for (const auto &arg: args) {
            strings_size += arg.size() + 1;
        }

        size_t total_size = argv_size + strings_size;

        char *block = new char[total_size];

        *argv = reinterpret_cast<char **>(block);

        char *strings = block + argv_size;

        for (int i = 0; i < *argc; ++i) {
            (*argv)[i] = strings;
            std::memcpy(strings, args[i].c_str(), args[i].size() + 1);
            strings += args[i].size() + 1;
        }
        (*argv)[*argc] = nullptr;

        return cmd;
    }

    ERR Cli::exec(CMD cmd, int argc, char **argv) {
        ERR err = ERR_Ok;
        switch (cmd) {
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
}
