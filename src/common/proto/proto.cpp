#include "proto.hpp"

namespace proto {
    Response *ParseResponse(Message *msg, ERR *err) {
        *err = ERR_Ok;
        const u8 *buf = msg->buf();
        auto type = static_cast<ResponseType>(*(buf + sizeof(size_t)));
        switch (type) {
            case RESP_OS_INFO:
                return new OsInfoResponse(buf, err);
            default:
                *err = ERR_Invalid_Response;
                return nullptr;
        }
    }
}
