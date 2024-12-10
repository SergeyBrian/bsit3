#ifndef BSIT_3_PROTO_HPP
#define BSIT_3_PROTO_HPP

#include "response.hpp"
#include "message.hpp"

namespace proto {
    Response *ParseResponse(Message *msg, ERR *err);
}

#endif
