#include "tcp.hpp"
#include "../../common/logging.hpp"

namespace server::tcp {
    std::vector<Server *> servers;
    bool cleanupSet = false;

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
    }

    void Server::RegisterHandler(proto::RequestType type, HandlerFunc handler) {
        m_handlers[type] = handler;
    }

    ERR Server::Start() {
        int res = WSAStartup(MAKEWORD(2, 2), &m_wsaData);
        if (res) {
            PRINT_ERROR("WSAStartup", WSAGetLastError());
            return winCodeToErr(WSAGetLastError());
        }

        OKAY("WSAStartup");
        sockaddr_in addr = {};
        SOCKET s = WSASocketW(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
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

            GetQueuedCompletionStatus(m_ioPort,
                                      &transferred,
                                      &key,
                                      &overlap,
                                      INFINITE);
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
        m_acceptSocket = WSASocketW(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
        std::memset(&m_clients[0].recvOverlap, 0, sizeof(OVERLAPPED));
        AcceptEx(m_clients[0].socket, m_acceptSocket, m_clients[0].recvBuf, 0, sizeof(sockaddr_in) + 16,
                 sizeof(sockaddr_in) + 16, nullptr, &m_clients[0].recvOverlap);
    }


    void Server::AddAcceptedConnection() {
        u32 key = 0;
        for (auto &client: m_clients) {
            if (client.socket != INVALID_SOCKET) {
                key++;
                continue;
            }
            u32 ip = 0;
            sockaddr_in *localAddr = nullptr;
            sockaddr_in *remoteAddr = nullptr;
            u32 localAddrSize = 0;
            u32 remoteAddrSize = 0;

            GetAcceptExSockaddrs(m_clients[0].recvBuf, m_clients[0].recvBufSize, sizeof(sockaddr_in) + 16,
                                 sizeof(sockaddr_in) + 16, reinterpret_cast<sockaddr **>(&localAddr),
                                 reinterpret_cast<LPINT>(&localAddrSize), reinterpret_cast<sockaddr **>(&remoteAddr),
                                 reinterpret_cast<LPINT>(&remoteAddrSize));
            if (remoteAddr) {
                ip = ntohl(remoteAddr->sin_addr.s_addr);
                LOG("Client %u.%u.%u.%u connected",
                    (ip >> 24) & 0xff, (ip >> 16) & 0xff, (ip >> 8) & 0xff, (ip) & 0xff);
                client.socket = m_acceptSocket;
                if (!CreateIoCompletionPort(reinterpret_cast<HANDLE>(client.socket),
                                            m_ioPort, key, 0)) {
                    PRINT_ERROR("CreateIoCompletionPort", WSAGetLastError());
                    break;
                }
                ScheduleRead(key);
                return;
            }
        }
        closesocket(m_acceptSocket);
        m_acceptSocket = INVALID_SOCKET;
    }

    void Server::ScheduleRead(u32 key) {
        WSABUF buf;
        Client &client = m_clients[key];
        buf.buf = reinterpret_cast<char *>(client.recvBuf + client.recvBufSize);
        buf.len = sizeof(client.recvBuf) - client.recvBufSize;
        std::memset(&client.recvOverlap, 0, sizeof(OVERLAPPED));
        client.recvFlags = 0;
        WSARecv(client.socket,
                &buf,
                1,
                nullptr,
                &client.recvFlags,
                &client.recvOverlap,
                nullptr);
    }

    void Server::ProcessEvent(ULONG_PTR key, OVERLAPPED *overlap, DWORD transferred) {
        Client &client = m_clients[key];
        if (&client.recvOverlap == overlap) {
            if (transferred == 0) {
                CancelIo(reinterpret_cast<HANDLE>(client.socket));
                PostQueuedCompletionStatus(m_ioPort, 0, key, &client.cancelOverlap);
                return;
            }
            client.recvBufSize += transferred;
            if (!proto::Message::ValidateBuff(client.recvBuf, client.recvBufSize)) {
                ScheduleRead(key);
                return;
            }
            ProcessMessage(client, proto::Message(client.recvBuf));
        } else if (&client.sendOverlap == overlap) {
            client.sentSize += transferred;
            if (client.sentSize < client.sendBufSize && transferred > 0) {
                ScheduleWrite(client);
                return;
            }
            ScheduleTimeout(key);
        } else if (&client.cancelOverlap == overlap) {
            closesocket(client.socket);
            std::memset(&m_clients[key], 0, sizeof(m_clients[key]));
            client.socket = INVALID_SOCKET;
            LOG("Client %lu disconnected", key);
        }
    }

    void Server::ProcessMessage(Client &client, const proto::Message &message) {
        proto::Request req(message.buf());
        proto::Response *resp = m_handlers[req.type](&req);
        SendResponse(client, resp);
        delete resp;
    }

    void Server::SendResponse(Client &client, proto::Response *resp) {
        proto::Message msg(resp);
        client.sendBufSize = msg.size();
        client.sentSize = 0;
        std::memcpy(client.sendBuf, msg.buf(), msg.size());
        ScheduleWrite(client);
    }

    void Server::ScheduleWrite(Client &client) {
        WSABUF buf{
                .len = client.sendBufSize - client.sentSize,
                .buf = reinterpret_cast<CHAR *>(client.sendBuf + client.sentSize),
        };
        std::memset(&client.sendOverlap, 0, sizeof(OVERLAPPED));
        WSASend(client.socket,
                &buf,
                1,
                nullptr,
                0,
                &client.sendOverlap,
                nullptr);
    }

    void Server::ScheduleTimeout(ULONG_PTR key) {
        Client &client = m_clients[key];
        // TODO: Timeout instead of disconnect
        CancelIo(reinterpret_cast<HANDLE>(client.socket));
        PostQueuedCompletionStatus(m_ioPort, 0, key, &client.cancelOverlap);
    }

    Server::~Server() {
        this->Cleanup();
    }

    void Server::Cleanup() {
        for (const auto &client: m_clients) {
            closesocket(client.socket);
        }
        WSACleanup();
    }
}
