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
}
