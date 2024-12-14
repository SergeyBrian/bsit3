#include "handlers.hpp"

#include "../os_utils.hpp"

namespace server::handlers {
void Init(tcp::Server *srv) {
    srv->RegisterHandler(proto::REQ_OS_INFO, HandleGetOsInfo);
    srv->RegisterHandler(proto::REQ_UPTIME, HandleGetUptime);
    srv->RegisterHandler(proto::REQ_TIME, HandleGetTime);
    srv->RegisterHandler(proto::REQ_DRIVES, HandleGetDrives);
    srv->RegisterHandler(proto::REQ_MEMORY, HandleGetMemory);
    srv->RegisterHandler(proto::REQ_RIGHTS, HandleGetRights);
    srv->RegisterHandler(proto::REQ_OWNER, HandleGetOwner);
}

proto::Response *HandleGetOsInfo(proto::Request *req) {
    return new proto::OsInfoResponse({
        .type = os_utils::get_type(),
        .version = os_utils::get_version(),
    });
}

proto::Response *HandleGetUptime(proto::Request *req) {
    return new proto::TimeResponse(os_utils::get_uptime_ms());
}

proto::Response *HandleGetTime(proto::Request *req) {
    return new proto::TimeResponse(os_utils::get_time_ms(),
                                   os_utils::get_timezone_hours());
}

proto::Response *HandleGetDrives(proto::Request *req) {
    return new proto::DrivesResponse(os_utils::get_drives());
}

proto::Response *HandleGetMemory(proto::Request *req) {
    return new proto::MemoryResponse(os_utils::get_meminfo());
}

proto::Response *HandleGetRights(proto::Request *req) {
    return new proto::RightsResponse(os_utils::get_access_info(req->arg));
}

proto::Response *HandleGetOwner(proto::Request *req) {
    return new proto::OwnerResponse(os_utils::get_owner_info(req->arg));
}
}  // namespace server::handlers
