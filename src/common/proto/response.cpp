#include "response.hpp"

namespace proto {
    const u8 *OsInfoResponse::pack(size_t *size) const {
        *size = sizeof(size);
        *size += sizeof(ResponseType);
        *size += sizeof(info.type);
        *size += sizeof(info.version.major) + sizeof(info.version.minor);
        auto buf = new u8[*size];
        auto tmp = buf;
        *reinterpret_cast<size_t *>(tmp) = *size;
        tmp += sizeof(*size);
        *tmp = RESP_OS_INFO;
        tmp += sizeof(ResponseType);
        *tmp = info.type;
        tmp += sizeof(info.type);
        *reinterpret_cast<u16 *>(tmp) = info.version.major;
        tmp += sizeof(info.version.major);
        *reinterpret_cast<u16 *>(tmp) = info.version.minor;

        return buf;
    }

    OsInfoResponse::OsInfoResponse(const u8 *buf, ERR *err) {
        auto size = *reinterpret_cast<const size_t *>(buf);
        buf += sizeof(size);
        buf += sizeof(ResponseType);
        info.type = static_cast<OSType>(*buf);
        buf += sizeof(info.type);
        info.version.major = *reinterpret_cast<const u16 *>(buf);
        buf += sizeof(info.version.major);
        info.version.minor = *reinterpret_cast<const u16 *>(buf);
        *err = ERR_Ok;
    }

    OsInfoResponse::OsInfoResponse(OSInfo info) : info(info){}
}
