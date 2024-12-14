#include "request.hpp"

#include "../str_utils.hpp"

namespace proto {
const u8 *Request::pack(usize *size) const {
    PackCtx ctx;
    ctx.push(type);
    if (!arg.empty()) {
        if constexpr (sizeof(wchar_t) == 4) {
            std::u16string utf16_bytes = utils::make_u16string(arg);
            ctx.push(utf16_bytes.data(),
                     utf16_bytes.size() * sizeof(utf16_bytes[0]));
        } else {
            ctx.push(arg.data(), arg.size() * sizeof(arg[0]));
        }
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
