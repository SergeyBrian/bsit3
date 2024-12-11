#ifndef BSIT_3_TCP_HPP
#define BSIT_3_TCP_HPP

#include "../../common/errors.hpp"
#include "../../common/proto/response.hpp"
#include "../../common/proto/request.hpp"

namespace connector :: tcp {
    proto::Response * exec(proto::Request *req, ERR *err, const std::string& host, u16 port);
}

#endif