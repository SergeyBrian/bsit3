#include "context.hpp"
#include "../../common/logging.hpp"

namespace connector::tcp {
    Context::Context(std::chrono::seconds timeout) : m_timeout(timeout) {
        WSAStartup(MAKEWORD(2,2), &m_wsaData);
    }

    Context::~Context() {
        closesocket(m_socket);
        WSACleanup();
    }

    bool Context::Expired() {
        return m_socket == INVALID_SOCKET || std::chrono::steady_clock::now() - m_lastConnTime > m_timeout;
    }

    ERR Context::Connect(const std::string &host, u16 port) {
        ERR err = ERR_Ok;
        addrinfo hints{};
        addrinfo *tmp;
        int res = 0;

        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        res = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &tmp);
        if (res) {
            PRINT_ERROR("getaddrinfo", res);
            return winCodeToErr(res);
        }

        addrinfo *addr;

        for (addr = tmp; addr; addr = addr->ai_next) {
            this->m_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
            if (this->m_socket == INVALID_SOCKET) {
                PRINT_ERROR("m_socket", WSAGetLastError());
                return winCodeToErr(WSAGetLastError());
            }

            res = connect(this->m_socket, addr->ai_addr, static_cast<int>(addr->ai_addrlen));
            if (res) {
                closesocket(this->m_socket);
                this->m_socket = INVALID_SOCKET;
                continue;
            }

            OKAY("Connected to server");
            break;
        }

        freeaddrinfo(addr);

        if (this->m_socket == INVALID_SOCKET) {
            WARN("Can't Connect to server");
            return ERR_Connect;
        }

        return err;
    }

    ERR Context::Send(proto::Message *msg) const {
        INFO("Sending request...");
        int res = send(m_socket, reinterpret_cast<const char *>(msg->buf()), static_cast<int>(msg->size()), 0);
        if (res == SOCKET_ERROR) {
            PRINT_ERROR("send", WSAGetLastError());
            return winCodeToErr(WSAGetLastError());
        }
        OKAY("Request sent");
        return ERR_Ok;
    }

    proto::Message Context::Receive(ERR *err) {
        INFO("Waiting for response...");
        char buf[MAX_MSG_SIZE];
        int res;

        do {
            res = recv(m_socket, buf, MAX_MSG_SIZE, 0);
        } while (res > 0);

        if (res < 0) {
            PRINT_ERROR("recv", WSAGetLastError());
            *err = winCodeToErr(WSAGetLastError());
            return {};
        }
        *err = ERR_Ok;
        // TODO: Pass received size to Message constructor to prevent memory overflow exploit
        return proto::Message(reinterpret_cast<const u8 *>(buf));
    }
}
