#include <cassert>
#include "tcp.hpp"

#include "../../common/logging.hpp"
#include "../../common/proto/encryption/encryption.hpp"

namespace server::tcp {
std::vector<Server *> servers;
bool cleanupSet = false;
const std::chrono::seconds TIMEOUT(20);

void TCPCleanup() {
    for (auto srv : servers) {
        srv->Cleanup();
    }
}

BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT ||
        signal == CTRL_BREAK_EVENT || signal == CTRL_LOGOFF_EVENT ||
        signal == CTRL_SHUTDOWN_EVENT) {
        OKAY("Graceful TCP server shutdown");
        TCPCleanup();
        ExitProcess(0);
    }
    return TRUE;
}

Server::Server(u16 port) {
    m_port = port;
    servers.push_back(this);
    if (!cleanupSet) {
        SetConsoleCtrlHandler(ConsoleHandler, TRUE);
    }
    proto::encryption::init();
}

void Server::RegisterHandler(proto::RequestType type, HandlerFunc handler) {
    assert(!m_handlers.contains(type) && "Handler already assigned");
    m_handlers[type] = handler;
}

void Server::TimeoutCheck() {
    INFO("Timeout check triggered");
    auto now = std::chrono::steady_clock::now();
    for (const auto &client : m_clients) {
        // First "client" is acutally the accept socket so skip it
        if (client.id == 0 || client.socket == INVALID_SOCKET) continue;
        if (now - client.last_activity > TIMEOUT) {
            ScheduleDisconnect(client.id);
        }
    }
}

ERR Server::Start() {
    int res = WSAStartup(MAKEWORD(2, 2), &m_wsaData);
    if (res) {
        PRINT_ERROR("WSAStartup", WSAGetLastError());
        return winCodeToErr(WSAGetLastError());
    }

    OKAY("WSAStartup");
    sockaddr_in addr = {};
    SOCKET s =
        WSASocketW(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
    m_ioPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    if (!m_ioPort) {
        PRINT_ERROR("CreateIoCompletionPort", WSAGetLastError());
        return winCodeToErr(WSAGetLastError());
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    res = bind(s, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr));
    if (res) {
        PRINT_ERROR("bind", WSAGetLastError());
        return winCodeToErr(WSAGetLastError());
    }
    res = listen(s, 1);
    if (res) {
        PRINT_ERROR("listen", WSAGetLastError());
        return winCodeToErr(WSAGetLastError());
    }

    OKAY("Server started listening on port %d", m_port);

    if (!CreateIoCompletionPort(reinterpret_cast<HANDLE>(s), m_ioPort, 0, 0)) {
        PRINT_ERROR("CreateIoCompletionPort", WSAGetLastError());
        return winCodeToErr(WSAGetLastError());
    }
    m_clients[0].socket = s;
    ScheduleAccept();

    OKAY("Server started");
    while (true) {
        DWORD transferred = 0;
        ULONG_PTR key = 0;
        OVERLAPPED *overlap;

        bool status = GetQueuedCompletionStatus(
            m_ioPort, &transferred, &key, &overlap, TIMEOUT.count() * 1000);
        if (!status) {
            TimeoutCheck();
            continue;
        }

        if (key == 0) {
            m_clients[0].recvBufSize += transferred;
            AddAcceptedConnection();
            ScheduleAccept();
            continue;
        }

        ProcessEvent(key, overlap, transferred);
    }

    return ERR_Ok;
}

void Server::ScheduleAccept() {
    m_acceptSocket =
        WSASocketW(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
    std::memset(&m_clients[0].recvOverlap, 0, sizeof(OVERLAPPED));
    AcceptEx(m_clients[0].socket, m_acceptSocket, m_clients[0].recvBuf, 0,
             sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, nullptr,
             &m_clients[0].recvOverlap);
}

void Server::AddAcceptedConnection() {
    TimeoutCheck();
    u32 key = 0;
    for (auto &client : m_clients) {
        if (client.socket != INVALID_SOCKET) {
            key++;
            continue;
        }
        client.id = key;
        client.last_activity = std::chrono::steady_clock::now();
        u32 ip = 0;
        sockaddr_in *localAddr = nullptr;
        sockaddr_in *remoteAddr = nullptr;
        u32 localAddrSize = 0;
        u32 remoteAddrSize = 0;

        GetAcceptExSockaddrs(m_clients[0].recvBuf, m_clients[0].recvBufSize,
                             sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
                             reinterpret_cast<sockaddr **>(&localAddr),
                             reinterpret_cast<LPINT>(&localAddrSize),
                             reinterpret_cast<sockaddr **>(&remoteAddr),
                             reinterpret_cast<LPINT>(&remoteAddrSize));
        if (remoteAddr) {
            ip = ntohl(remoteAddr->sin_addr.s_addr);
            LOG("Client %d (%u.%u.%u.%u) connected", key, (ip >> 24) & 0xff,
                (ip >> 16) & 0xff, (ip >> 8) & 0xff, (ip) & 0xff);
            client.socket = m_acceptSocket;
            if (!CreateIoCompletionPort(reinterpret_cast<HANDLE>(client.socket),
                                        m_ioPort, key, 0)) {
                PRINT_ERROR("CreateIoCompletionPort", WSAGetLastError());
                break;
            }
            proto::encryption::g_instance->CreateSymmetricKey(client.id);
            ScheduleRead(key);
            return;
        }
    }
    closesocket(m_acceptSocket);
    m_acceptSocket = INVALID_SOCKET;
    WARN("Failed to connect. Client pool full");
}

void Server::ScheduleRead(u32 key, bool reset) {
    WSABUF buf;
    Client &client = m_clients[key];
    if (reset) {
        client.recvBufSize = 0;
    }
    buf.buf = reinterpret_cast<char *>(client.recvBuf + client.recvBufSize);
    buf.len = sizeof(client.recvBuf) - client.recvBufSize;
    std::memset(&client.recvOverlap, 0, sizeof(OVERLAPPED));
    client.recvFlags = 0;
    int res = WSARecv(client.socket, &buf, 1, nullptr, &client.recvFlags,
                      &client.recvOverlap, nullptr);
    if (res == SOCKET_ERROR) {
        if (WSAGetLastError() != ERROR_IO_PENDING) {
            PRINT_ERROR("WSARecv", WSAGetLastError());
        }
    }
}

void Server::ProcessEvent(ULONG_PTR key, OVERLAPPED *overlap,
                          DWORD transferred) {
    Client &client = m_clients[key];
    if (&client.recvOverlap == overlap) {
        client.last_activity = std::chrono::steady_clock::now();
        INFO("Recv overlap triggered");
        if (transferred == 0) {
            CancelIo(reinterpret_cast<HANDLE>(client.socket));
            PostQueuedCompletionStatus(m_ioPort, 0, key, &client.cancelOverlap);
            return;
        }
        client.recvBufSize += transferred;
        if (!proto::Message::ValidateBuff(client.recvBuf, client.recvBufSize)) {
            INFO("Message invalid");
            ScheduleRead(key);
            return;
        }
        ProcessMessage(client, proto::Message(client.id, client.recvBuf));
        INFO("ProcessMessage done");
        client.recvBufSize = 0;
    } else if (&client.sendOverlap == overlap) {
        INFO("Send overlap triggered");
        client.sentSize += transferred;
        if (client.sentSize < client.sendBufSize && transferred > 0) {
            INFO("Written %lu bytes. %llu more to be sent", transferred,
                 client.sendBufSize - client.sentSize);
            ScheduleWrite(client);
            return;
        }
        INFO("Written %lu bytes. That's it", transferred);
        ScheduleRead(client.id, true);
    } else if (&client.cancelOverlap == overlap) {
        INFO("Cancel overlap triggered");
        if (client.socket == INVALID_SOCKET) {
            return;
        }
        closesocket(client.socket);
        std::memset(&m_clients[key], 0, sizeof(m_clients[key]));
        client.socket = INVALID_SOCKET;
        LOG("Client %lu disconnected", key);
    }
}

void Server::ProcessMessage(Client &client, const proto::Message &message) {
    if (message.type() == proto::MESSAGE_KEY_REQUEST) {
        INFO("Received key request");
        DWORD size;
        auto buf = proto::encryption::g_instance->ExportSymmetricKey(
            client.id, &size, message.buf(), message.size());
        proto::Message msg(proto::MESSAGE_KEY_RESPONSE, buf, size,
                           proto::MESSAGE_ENCRYPTION_ASYMMETRIC);

        client.sendBufSize = msg.size();
        client.sentSize = 0;
        std::memcpy(client.sendBuf, msg.buf(), msg.size());
        INFO("Sent message with key of size %llu", msg.size());
        utils::dump_memory(msg.buf(), msg.size());
        ScheduleWrite(client);
        INFO("Schedule write key done.");
        ScheduleRead(client.id, true);
        INFO("Schedule read next message done.");
        return;
    }

    proto::Request req(message.buf());
    INFO("Received request %d", req.type);
    if (!m_handlers.contains(req.type)) {
        WARN("Unknown request");
        return;
    }
    proto::Response *resp = m_handlers[req.type](&req);
    SendResponse(client, resp);
    delete resp;
}

void Server::SendResponse(Client &client, proto::Response *resp) {
    proto::Message msg(resp, proto::MESSAGE_ENCRYPTION_SYMMETRIC, client.id);
    client.sendBufSize = msg.size();
    client.sentSize = 0;
    std::memcpy(client.sendBuf, msg.buf(), msg.size());
    INFO("Sent message of size %llu", msg.size());
    utils::dump_memory(msg.buf(), msg.size());
    ScheduleWrite(client);
}

void Server::ScheduleWrite(Client &client) {
    WSABUF buf{
        .len = static_cast<ULONG>(client.sendBufSize - client.sentSize),
        .buf = reinterpret_cast<CHAR *>(client.sendBuf + client.sentSize),
    };
    std::memset(&client.sendOverlap, 0, sizeof(OVERLAPPED));
    int res = WSASend(client.socket, &buf, 1, nullptr, 0, &client.sendOverlap,
                      nullptr);
    if (res == SOCKET_ERROR) {
        if (WSAGetLastError() != ERROR_IO_PENDING) {
            PRINT_ERROR("WSASend", WSAGetLastError());
        }
    }
}

void Server::ScheduleDisconnect(ULONG_PTR key) {
    assert(key != 0 && "Attempted to disconnect the accept socket");
    INFO("Disconnect scheduled for client %lu", key);
    Client &client = m_clients[key];
    if (!CancelIo(reinterpret_cast<HANDLE>(client.socket))) {
        PRINT_ERROR("CancelIO", WSAGetLastError());
        return;
    }

    PostQueuedCompletionStatus(m_ioPort, 0, key, &client.cancelOverlap);
}

Server::~Server() { this->Cleanup(); }

void Server::Cleanup() {
    for (const auto &client : m_clients) {
        closesocket(client.socket);
    }
    WSACleanup();
}
}  // namespace server::tcp
