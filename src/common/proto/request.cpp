#include "request.hpp"

#include <utility>

namespace proto {
    const u8 *Request::pack(size_t *size) const {
        size_t s = sizeof(type);
        size_t arg_size = 0;

        if (!arg.empty()) {
            arg_size = arg.size() * sizeof(arg[0]);
            s += arg_size;
        }

        s += sizeof(s);
        *size = s;

        auto buf = new u8[s];
        u8 *tmp_buf = buf;

        *reinterpret_cast<size_t *>(tmp_buf) = s;
        tmp_buf += sizeof(s);
        *tmp_buf = type;
        tmp_buf += sizeof(type);
        if (arg_size) {
            std::memcpy(tmp_buf, arg.data(), arg_size);
        }

        return buf;
    }

    Request::Request(RequestType type) : type(type) {}

    Request::Request(RequestType type, std::wstring arg) : type(type), arg(std::move(arg)) {}

    Request::Request(const u8 *buf) {
        auto size = *reinterpret_cast<const size_t *>(buf);
        buf += sizeof(size);
        type = static_cast<RequestType>(*buf);
        buf += sizeof(type);

        size -= sizeof(size) + sizeof(type);
        if (size > 0) {
            size_t num_wchars = size / sizeof(wchar_t);
            auto wbuf = reinterpret_cast<const wchar_t*>(buf);

            arg.assign(wbuf, num_wchars);
        }
    }
}
