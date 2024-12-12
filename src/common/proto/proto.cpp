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
            case RESP_DRIVES:
                return new DrivesResponse(&ctx, err);
            case RESP_MEMORY:
                return new MemoryResponse(&ctx, err);
            case RESP_RIGHTS:
                return new RightsResponse(&ctx, err);
            case RESP_OWNER:
                return new OwnerResponse(&ctx, err);
            default:
                *err = ERR_Invalid_Response;
                return nullptr;
        }
    }
}
