#include "response.hpp"

namespace proto {
    const u8 *OsInfoResponse::pack(size_t *size) const {
        PackCtx ctx;
        ctx.push(RESP_OS_INFO);
        ctx.push(info.type);
        ctx.push(info.version.major);
        ctx.push(info.version.minor);

        return ctx.pack(size);
    }

    OsInfoResponse::OsInfoResponse(PackCtx *ctx, ERR *err) {
        info.type = ctx->pop<OSType>();
        info.version.major = ctx->pop<u16>();
        info.version.minor = ctx->pop<u16>();
        *err = ERR_Ok;
    }

    OsInfoResponse::OsInfoResponse(OSInfo info) : info(info){}

    const u8 *TimeResponse::pack(size_t *size) const {
        PackCtx ctx;
        ctx.push(RESP_TIME);
        ctx.push(time_ms);

        return ctx.pack(size);
    }

    TimeResponse::TimeResponse(PackCtx *ctx, ERR *err) {
        time_ms = ctx->pop<u64>();

        *err = ERR_Ok;
    }

    TimeResponse::TimeResponse(u64 uptime) : time_ms(uptime){}
}
