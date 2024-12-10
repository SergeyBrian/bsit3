#include "tcp.hpp"

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "../../common/logging.hpp"
#include "context.hpp"
#include "../../common/proto/message.hpp"
#include "../../common/proto/proto.hpp"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


namespace connector :: tcp {
    Context *ctx;

    ERR reconnect(const std::string& host, u16 port) {
        delete ctx;
        ctx = new Context();
        ERR err = ctx->Connect(host, port);

        return err;
    }

    proto::Response * exec(proto::Request *req, ERR *err, const std::string& host, u16 port) {
        *err = ERR_Ok;
        if (!ctx || ctx->Expired()) {
            INFO("Reconnecting to the server...");
            *err = reconnect(host, port);
            if (*err != ERR_Ok) {
                WARN("Error connecting to server: %s", errorText[err]);
                return nullptr;
            }
        }

        int res = 0;

        auto msg = proto::Message(req);
        *err = ctx->Send(&msg);
        if (*err != ERR_Ok) {
            return nullptr;
        }
        proto::Message resp_msg = ctx->Receive(err);
        proto::Response *resp = proto::ParseResponse(&resp_msg, err);

        return resp;
    }
}
