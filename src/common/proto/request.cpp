#include "request.hpp"

#include <utility>

namespace proto {
const u8 *Request::pack(usize *size) const {
    PackCtx ctx;
    ctx.push(type);
    if (!arg.empty()) {
        ctx.push(arg.data(), arg.size() * sizeof(arg[0]));
    }
    return ctx.pack(size);
}

Request::Request(RequestType type) : type(type) {}

Request::Request(RequestType type, std::wstring arg)
    : type(type), arg(std::move(arg)) {}

Request::Request(const u8 *buf) {
    PackCtx ctx(buf);
    type = ctx.pop<RequestType>();
    if (type != REQ_RIGHTS && type != REQ_OWNER) return;
    usize arg_size;
    auto wbuf = ctx.pop<wchar_t>(&arg_size);
    arg.assign(wbuf, arg_size / sizeof(wchar_t));
}
}  // namespace proto
