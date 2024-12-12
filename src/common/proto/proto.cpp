#include "proto.hpp"

namespace proto {
    Response *ParseResponse(Message *msg, ERR *err) {
        *err = ERR_Ok;
        PackCtx ctx(msg->buf());
        auto type = ctx.pop<ResponseType>();
        switch (type) {
            case RESP_OS_INFO:
                return new OsInfoResponse(&ctx, err);
            case RESP_TIME:
                return new TimeResponse(&ctx, err);
            default:
                *err = ERR_Invalid_Response;
                return nullptr;
        }
    }
}
