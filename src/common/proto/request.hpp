#ifndef REQUESTS_HPP
#define REQUESTS_HPP


#include <string>
#include "../alias.hpp"
#include "packable.hpp"

namespace proto {
    enum RequestType : u8 {
        REQ_OS_INFO,
    };

    struct Request : Packable {
        RequestType type;
        std::wstring arg;

        const u8 *pack(size_t *size) const override;
        explicit Request(RequestType type);
        Request(RequestType type, std::wstring arg);
    };
}


#endif