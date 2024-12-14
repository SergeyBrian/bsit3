#ifndef BSIT_3_CONTEXT_HPP
#define BSIT_3_CONTEXT_HPP

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <unistd.h>

#define INVALID_SOCKET 0
#endif

#include <chrono>

#include "../../common/alias.hpp"
#include "../../common/errors.hpp"
#include "../../common/proto/message.hpp"

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")
#endif

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
#ifdef _WIN32
    WSADATA m_wsaData = {};
    SOCKET m_socket = INVALID_SOCKET;
#else
    int m_socket = 0;
#endif

    std::chrono::time_point<std::chrono::steady_clock> m_lastConnTime;
    std::chrono::seconds m_timeout;
};
}  // namespace connector::tcp

#endif
