#ifndef BSIT_3_HANDLERS_HPP
#define BSIT_3_HANDLERS_HPP

#include "../../common/alias.hpp"
#include "../../common/proto/request.hpp"
#include "../../common/proto/response.hpp"
#include "tcp.hpp"

namespace server::handlers {
proto::Response *HandleGetOsInfo(proto::Request *req);
proto::Response *HandleGetUptime(proto::Request *req);
proto::Response *HandleGetTime(proto::Request *req);
proto::Response *HandleGetDrives(proto::Request *req);
proto::Response *HandleGetMemory(proto::Request *req);
proto::Response *HandleGetRights(proto::Request *req);
proto::Response *HandleGetOwner(proto::Request *req);

void Init(tcp::Server *srv);
}  // namespace server::handlers

#endif
