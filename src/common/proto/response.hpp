#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include "../alias.hpp"
#include "../data.hpp"
#include "../errors.hpp"
#include "packable.hpp"

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

    const u8 *pack(usize *size) const override;

    OsInfoResponse(PackCtx *ctx, ERR *err);

    explicit OsInfoResponse(OSInfo info);

    ~OsInfoResponse() override = default;
};

struct TimeResponse : Response {
    u64 time_ms = 0;

    const u8 *pack(usize *size) const override;

    TimeResponse(PackCtx *ctx, ERR *err);

    explicit TimeResponse(u64 uptime);

    ~TimeResponse() override = default;
};

struct DrivesResponse : Response {
    std::vector<DriveInfo> drives;

    const u8 *pack(usize *size) const override;

    DrivesResponse(PackCtx *ctx, ERR *err);

    explicit DrivesResponse(const std::vector<DriveInfo> &drives);

    ~DrivesResponse() override = default;
};

struct MemoryResponse : Response {
    MemInfo mem_info{};

    const u8 *pack(usize *size) const override;

    MemoryResponse(PackCtx *ctx, ERR *err);

    explicit MemoryResponse(MemInfo mem_info);

    ~MemoryResponse() override = default;
};

struct RightsResponse : Response {
    AccessRightsInfo rights_info{};

    const u8 *pack(usize *size) const override;

    RightsResponse(PackCtx *ctx, ERR *err);

    explicit RightsResponse(AccessRightsInfo rights_info);

    ~RightsResponse() override = default;
};

struct OwnerResponse : Response {
    OwnerInfo info{};

    const u8 *pack(usize *size) const override;

    OwnerResponse(PackCtx *ctx, ERR *err);

    explicit OwnerResponse(OwnerInfo info);

    ~OwnerResponse() override = default;
};
}  // namespace proto

#endif
