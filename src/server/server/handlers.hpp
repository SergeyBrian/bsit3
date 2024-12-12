#ifndef BSIT_3_HANDLERS_HPP
#define BSIT_3_HANDLERS_HPP

#include "../../common/alias.hpp"
#include "../../common/proto/response.hpp"
#include "../../common/proto/request.hpp"
#include "tcp.hpp"

namespace server::handlers {
    proto::Response *HandleGetOsInfo(proto::Request *req);
    proto::Response *HandleGetUptime(proto::Request *req);

    void Init(tcp::Server *srv);
}

#endif
