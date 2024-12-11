#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../alias.hpp"
#include "packable.hpp"
#include "../errors.hpp"
#include "../data.hpp"

namespace proto {
    enum ResponseType : u8 {
        RESP_OS_INFO,
    };

    struct Response : Packable {
        ERR err = ERR_Ok;
        virtual ~Response() {};
    };

    struct OsInfoResponse : Response {
        OSInfo info{};

        const u8 *pack(size_t *size) const override;

        OsInfoResponse(const u8 *buf, ERR *err);
        OsInfoResponse(OSInfo info);
        ~OsInfoResponse() override {};
    };
}

#endif
