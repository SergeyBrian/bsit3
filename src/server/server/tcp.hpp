#ifndef BSIT_3_TCP_HPP
#define BSIT_3_TCP_HPP

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <mswsock.h>

#include <unordered_map>

#include "../../common/alias.hpp"
#include "../../common/proto/message.hpp"
#include "../../common/proto/request.hpp"
#include "../../common/proto/response.hpp"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")

#define MAX_CLIENTS (100)
#define MAX_BUF_SIZE 512

typedef proto::Response *(*HandlerFunc)(proto::Request *);

namespace server::tcp {
struct Client {
    SOCKET socket = INVALID_SOCKET;
    u8 recvBuf[MAX_BUF_SIZE] = {};
    u8 sendBuf[MAX_BUF_SIZE] = {};

    usize recvBufSize = 0;
    usize sendBufSize = 0;
    usize sentSize = 0;

    OVERLAPPED recvOverlap = {};
    OVERLAPPED sendOverlap = {};
    OVERLAPPED cancelOverlap = {};

    DWORD recvFlags = 0;
};

class Server {
public:
    explicit Server(u16 port);
    ~Server();

    void RegisterHandler(proto::RequestType type, HandlerFunc handler);

    ERR Start();

    void Cleanup();

private:
    u16 m_port;
    std::unordered_map<proto::RequestType, HandlerFunc> m_handlers;

    Client m_clients[MAX_CLIENTS + 1] = {};
    SOCKET m_acceptSocket = INVALID_SOCKET;
    HANDLE m_ioPort = nullptr;

    WSADATA m_wsaData = {};

    void ScheduleAccept();

    void ScheduleRead(u32 key);

    void AddAcceptedConnection();

    void ProcessEvent(ULONG_PTR key, OVERLAPPED *overlap, DWORD transferred);

    void ProcessMessage(Client &client, const proto::Message &message);

    void SendResponse(Client &client, proto::Response *resp);

    void ScheduleWrite(Client &client);

    void ScheduleTimeout(ULONG_PTR key);
};
}  // namespace server::tcp

#endif
