#ifndef REQUESTS_HPP
#define REQUESTS_HPP

#include <string>

#include "../alias.hpp"
#include "packable.hpp"

namespace proto {
enum RequestType : u8 {
    REQ_OS_INFO,
    REQ_TIME,
    REQ_UPTIME,
    REQ_MEMORY,
    REQ_DRIVES,
    REQ_RIGHTS,
    REQ_OWNER,
};

struct Request : Packable {
    RequestType type;
    std::wstring arg;

    std::unique_ptr<const u8[]> pack(usize *size) const override;
    explicit Request(RequestType type);
    Request(RequestType type, std::wstring arg);
    explicit Request(const u8 *buf);
};
}  // namespace proto

#endif
