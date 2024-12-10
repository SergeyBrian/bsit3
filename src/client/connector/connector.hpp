#ifndef CONNECTOR_HPP
#define CONNECTOR_HPP

#include <string>

#include "../../common/alias.hpp"
#include "../../common/errors.hpp"
#include "../../common/proto/request.hpp"
#include "../../common/proto/response.hpp"
#include "../../common/data.hpp"

namespace connector {
    class Connector {
    public:
        Connector(const std::string &host, u16 port);

        bool canConnect() const;

        bool checkConnection() const;

        void setServer(std::string host, u16 port);

        std::string getHostStr();

        ERR getOsInfo(OSInfo *res) const;

    private:
        std::string m_host;
        u16 m_port;
        bool m_canConnect;

        proto::Response *exec(proto::Request *req, ERR *err) const;
    };
}

#endif
