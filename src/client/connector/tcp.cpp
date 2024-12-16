#include "tcp.hpp"

#include "../../common/logging.hpp"
#include "../../common/proto/encryption/encryption.hpp"
#include "../../common/proto/message.hpp"
#include "../../common/proto/proto.hpp"

namespace connector::tcp {
Context *ctx;

void disconnect() {
    INFO("Disconnecting");
    delete ctx;
    ctx = nullptr;
}

ERR reconnect(const std::string &host, u16 port) {
    INFO("Removing old context");
    delete ctx;
    OKAY("Done.");
    INFO("Creating new context");
    ctx = new Context();
    OKAY("Done");
    ERR err = ctx->Connect(host, port);
    if (err != ERR_Ok) {
        return err;
    }

    DWORD size;
    auto buf = proto::encryption::g_instance->ExportPublicKey(&size);
    proto::Message msg(proto::MESSAGE_KEY_REQUEST, buf, size,
                       proto::MESSAGE_ENCRYPTION_NONE);
    INFO("Requesting key...");
    err = ctx->Send(&msg);
    if (err != ERR_Ok) {
        return err;
    }
    OKAY("Key request sent");

    INFO("Receiving key response...");
    proto::Message resp_msg = ctx->Receive(&err);
    if (err != ERR_Ok) {
        return err;
    }
    OKAY("Key received");

    proto::encryption::g_instance->ImportSymmetricKey(1, resp_msg.buf(),
                                                      resp_msg.size());

    return err;
}

proto::Response *exec(proto::Request *req, ERR *err, const std::string &host,
                      u16 port) {
    *err = ERR_Ok;
    if (!ctx || ctx->Expired()) {
        INFO("Reconnecting to the server...");
        *err = reconnect(host, port);
        if (*err != ERR_Ok) {
            WARN("Error connecting to server: %s", errorText[*err]);
            return nullptr;
        }
    }

    int res = 0;

    auto msg = proto::Message(req, proto::MESSAGE_ENCRYPTION_SYMMETRIC, 1);
    *err = ctx->Send(&msg);
    if (*err != ERR_Ok) {
        return nullptr;
    }
    proto::Message resp_msg = ctx->Receive(err);
    if (*err != ERR_Ok) {
        return nullptr;
    }
    proto::Response *resp = proto::ParseResponse(&resp_msg, err);

    return resp;
}
}  // namespace connector::tcp
