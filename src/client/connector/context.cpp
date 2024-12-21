#include "context.hpp"

#include "../../common/logging.hpp"

namespace connector::tcp {
Context::Context(u32 cid, std::chrono::seconds timeout) : m_timeout(timeout), m_id(cid) {
#ifdef _WIN32
    WSAStartup(MAKEWORD(2, 2), &m_wsaData);
#endif
}

Context::~Context() {
#ifdef _WIN32
    closesocket(m_socket);
    WSACleanup();
#else
    close(m_socket);
#endif
}

bool Context::Expired() {
    return m_socket == INVALID_SOCKET ||
           std::chrono::steady_clock::now() - m_lastConnTime > m_timeout;
}

#ifdef _WIN32
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
        this->m_socket =
            socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if (this->m_socket == INVALID_SOCKET) {
            PRINT_ERROR("m_socket", WSAGetLastError());
            return winCodeToErr(WSAGetLastError());
        }

        res = connect(this->m_socket, addr->ai_addr,
                      static_cast<int>(addr->ai_addrlen));
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
        return ERR_Connect;
    }

    return err;
}
#else
ERR Context::Connect(const std::string &host, u16 port) {
    struct hostent *h = gethostbyname(host.c_str());
    sockaddr_in send_sock_addr;
    bzero(reinterpret_cast<char *>(&send_sock_addr), sizeof(send_sock_addr));
    send_sock_addr.sin_family = AF_INET;
    send_sock_addr.sin_addr.s_addr =
        inet_addr(inet_ntoa(*reinterpret_cast<in_addr *>(*h->h_addr_list)));
    send_sock_addr.sin_port = htons(port);
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    INFO("Trying to connect");
    int status =
        connect(m_socket, reinterpret_cast<sockaddr *>(&send_sock_addr),
                sizeof(send_sock_addr));
    if (status < 0) {
        return ERR_Connect;
    }

    return ERR_Ok;
}
#endif

#ifdef _WIN32
ERR Context::Send(proto::Message *msg) {
    m_lastConnTime = std::chrono::steady_clock::now();
    INFO("Sending request...");
    int res = send(m_socket, reinterpret_cast<const char *>(msg->buf()),
                   static_cast<int>(msg->size()), 0);
    if (res == SOCKET_ERROR) {
        PRINT_ERROR("send", WSAGetLastError());
        return winCodeToErr(WSAGetLastError());
    } else {
        INFO("Succesfully sent %d/%llu bytes!", res, msg->size());
    }
    utils::dump_memory(msg->buf(), msg->size());
    OKAY("Request sent");
    return ERR_Ok;
}
#else
ERR Context::Send(proto::Message *msg) const {
    m_lastConnTime = std::chrono::steady_clock::now();
    INFO("Sending request...");
    int res = send(m_socket, reinterpret_cast<const char *>(msg->buf()),
                   static_cast<int>(msg->size()), 0);
    utils::dump_memory(msg->buf(), msg->size());
    // TODO: check errors
    OKAY("Request sent");
    return ERR_Ok;
}
#endif

#ifdef _WIN32
proto::Message Context::Receive(ERR *err) {
    INFO("Waiting for response...");
    char buf[MAX_MSG_SIZE];
    int res;
    usize res_size = 0;

    do {
        res = recv(m_socket, buf, MAX_MSG_SIZE, 0);
        res_size += res;
        INFO("Received %d bytes", res);
        if (proto::Message::ValidateBuff(reinterpret_cast<const u8 *>(buf), res_size)) {
            OKAY("Valid message");
            break;
        }
    } while (res > 0);
    OKAY("Receiving finished");

    if (res < 0) {
        PRINT_ERROR("recv", WSAGetLastError());
        *err = winCodeToErr(WSAGetLastError());
        return {};
    }
    *err = ERR_Ok;
    // TODO: Pass received size to Message constructor to prevent memory
    // overflow exploit
    return proto::Message(m_id, reinterpret_cast<const u8 *>(buf));
}
#else
proto::Message Context::Receive(ERR *err) {
    INFO("Waiting for response...");
    char buf[MAX_MSG_SIZE];
    int res;

    do {
        res = recv(m_socket, buf, MAX_MSG_SIZE, 0);
    } while (res > 0);

    // TODO: check error
    *err = ERR_Ok;
    // TODO: Pass received size to Message constructor to prevent memory
    // overflow exploit
    return proto::Message(reinterpret_cast<const u8 *>(buf));
}
#endif
}  // namespace connector::tcp
