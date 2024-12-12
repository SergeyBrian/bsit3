#include "response.hpp"

#include <utility>

namespace proto {
    const u8 *OsInfoResponse::pack(size_t *size) const {
        PackCtx ctx;
        ctx.push(RESP_OS_INFO);
        ctx.push(info.type);
        ctx.push(info.version.major);
        ctx.push(info.version.minor);

        return ctx.pack(size);
    }

    OsInfoResponse::OsInfoResponse(PackCtx *ctx, ERR *err) {
        info.type = ctx->pop<OSType>();
        info.version.major = ctx->pop<u16>();
        info.version.minor = ctx->pop<u16>();
        *err = ERR_Ok;
    }

    OsInfoResponse::OsInfoResponse(OSInfo info) : info(info) {}

    const u8 *TimeResponse::pack(size_t *size) const {
        PackCtx ctx;
        ctx.push(RESP_TIME);
        ctx.push(time_ms);

        return ctx.pack(size);
    }

    TimeResponse::TimeResponse(PackCtx *ctx, ERR *err) {
        time_ms = ctx->pop<u64>();

        *err = ERR_Ok;
    }

    TimeResponse::TimeResponse(u64 uptime) : time_ms(uptime) {}

    const u8 *DrivesResponse::pack(size_t *size) const {
        PackCtx ctx;
        ctx.push(RESP_DRIVES);
        ctx.push(drives.size());
        for (const auto &drive: drives) {
            ctx.push(drive.type);
            ctx.push(drive.free_bytes);
            ctx.push(drive.name.data(), drive.name.size() * sizeof(drive.name[0]));
        }

        return ctx.pack(size);
    }

    DrivesResponse::DrivesResponse(PackCtx *ctx, ERR *err) {
        auto count = ctx->pop<size_t>();
        for (u64 i = 0; i < count; i++) {
            DriveInfo di;
            di.type = ctx->pop<DriveType>();
            di.free_bytes = ctx->pop<u64>();
            size_t name_size;
            auto name = ctx->pop<char>(&name_size);
            di.name.assign(name, name_size);
            drives.push_back(di);
        }

        *err = ERR_Ok;
    }

    DrivesResponse::DrivesResponse(const std::vector<DriveInfo> &drives) : drives(drives) {}

    const u8 *MemoryResponse::pack(size_t *size) const {
        PackCtx ctx;
        ctx.push(RESP_MEMORY);
        ctx.push(mem_info.free_bytes);
        ctx.push(mem_info.total_bytes);

        return ctx.pack(size);
    }

    MemoryResponse::MemoryResponse(PackCtx *ctx, ERR *err) {
        mem_info.free_bytes = ctx->pop<u64>();
        mem_info.total_bytes = ctx->pop<u64>();

        *err = ERR_Ok;
    }

    MemoryResponse::MemoryResponse(MemInfo mem_info) : mem_info(mem_info) {}

    const u8 *RightsResponse::pack(size_t *size) const {
        PackCtx ctx;
        ctx.push(RESP_RIGHTS);
        ctx.push(rights_info.entries.size());
        for (const auto &entry : rights_info.entries) {
            ctx.push(entry.accessMask);
            ctx.push(entry.aceType);
            ctx.push(entry.scope);
            ctx.push(entry.sid.data(), entry.sid.size());
        }

        return ctx.pack(size);
    }

    RightsResponse::RightsResponse(PackCtx *ctx, ERR *err) {
        auto count = ctx->pop<size_t>();
        for (u64 i = 0; i < count; i++) {
            AccessControlEntry entry{};
            entry.accessMask = ctx->pop<u32>();
            entry.aceType = ctx->pop<AceType>();
            entry.scope = ctx->pop<Scope>();
            size_t sid_size;
            u8 *sid = ctx->pop<u8>(&sid_size);
            std::memcpy(entry.sid.data(), sid, entry.sid.size());
            rights_info.entries.push_back(entry);
        }

        *err = ERR_Ok;
    }

    RightsResponse::RightsResponse(AccessRightsInfo rights_info) : rights_info(std::move(rights_info)) {}

    const u8 *OwnerResponse::pack(size_t *size) const {
        PackCtx ctx;
        ctx.push(RESP_OWNER);
        ctx.push(info.ownerDomain.data(), info.ownerDomain.size());
        ctx.push(info.ownerName.data(), info.ownerName.size());

        return ctx.pack(size);
    }

    OwnerResponse::OwnerResponse(PackCtx *ctx, ERR *err) {
        size_t name_size;
        auto domainName = ctx->pop<char>(&name_size);
        info.ownerDomain.assign(domainName, name_size);
        auto name = ctx->pop<char>(&name_size);
        info.ownerName.assign(name, name_size);

        *err = ERR_Ok;
    }

    OwnerResponse::OwnerResponse(OwnerInfo info) : info(std::move(info)) {}
}
