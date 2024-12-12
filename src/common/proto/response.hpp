#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../alias.hpp"
#include "packable.hpp"
#include "../errors.hpp"
#include "../data.hpp"

namespace proto {
    enum ResponseType : u8 {
        RESP_OS_INFO,
        RESP_TIME,
        RESP_MEMORY,
        RESP_DRIVES,
        RESP_RIGHTS,
        RESP_OWNER,
    };

    struct Response : Packable {
        ERR err = ERR_Ok;
        virtual ~Response() = default;
    };

    struct OsInfoResponse : Response {
        OSInfo info{};

        const u8 *pack(size_t *size) const override;

        OsInfoResponse(PackCtx *ctx, ERR *err);
        explicit OsInfoResponse(OSInfo info);
        ~OsInfoResponse() override = default;
    };

    struct TimeResponse : Response {
        u64 time_ms = 0;

        const u8 *pack(size_t *size) const override;

        TimeResponse(PackCtx *ctx, ERR *err);
        explicit TimeResponse(u64 uptime);
        ~TimeResponse() override = default;
    };
}

#endif
