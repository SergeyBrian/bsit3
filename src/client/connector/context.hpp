#ifndef BSIT_3_CONTEXT_HPP
#define BSIT_3_CONTEXT_HPP


#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <chrono>
#include "../../common/errors.hpp"
#include "../../common/alias.hpp"
#include "../../common/proto/message.hpp"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

namespace connector::tcp {
    constexpr auto DefaultTimeout = std::chrono::seconds(20);
    class Context {
    public:
        explicit Context(std::chrono::seconds timeout = DefaultTimeout);
        ~Context();

        ERR Connect(const std::string &host, u16 port);

        ERR Send(proto::Message *msg) const;

        bool Expired();

        proto::Message Receive(ERR *err);

    private:
        WSADATA m_wsaData = {};
        SOCKET m_socket = INVALID_SOCKET;

        std::chrono::time_point<std::chrono::steady_clock> m_lastConnTime;
        std::chrono::seconds m_timeout;
    };
}


#endif
