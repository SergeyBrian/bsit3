#ifndef BSIT_3_SERVER_HPP
#define BSIT_3_SERVER_HPP

#include "../../common/alias.hpp"

namespace server {

    class Server {
    public:
        explicit Server(u16 port);
        void run();
    private:
        u16 m_port;
    };

}

#endif
