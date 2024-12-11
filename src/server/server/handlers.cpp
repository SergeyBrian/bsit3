#include "handlers.hpp"

#include "../os_utils.hpp"

namespace server::handlers {
    proto::Response *HandleGetOsInfo(proto::Request *req) {
        return new proto::OsInfoResponse({
            .type = os_utils::get_type(),
            .version = os_utils::get_version(),
        });
    }

    void Init(tcp::Server *srv) {
        srv->RegisterHandler(proto::REQ_OS_INFO, HandleGetOsInfo);
    }
}