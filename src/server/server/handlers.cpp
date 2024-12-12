#include "handlers.hpp"

#include "../os_utils.hpp"

namespace server::handlers {
    void Init(tcp::Server *srv) {
        srv->RegisterHandler(proto::REQ_OS_INFO, HandleGetOsInfo);
        srv->RegisterHandler(proto::REQ_UPTIME, HandleGetUptime);
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
}