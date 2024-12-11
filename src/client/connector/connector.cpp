#include "connector.hpp"

#include <utility>
#include "../../common/logging.hpp"
#include "tcp.hpp"


namespace connector {
    Connector::Connector(const std::string &host, u16 port) {
        this->m_host = host;
        this->m_port = port;

        if (host.empty() && port == 0) {
            m_canConnect = false;
            OKAY("Initialized connector without initial host and port");
            return;
        }
        OKAY("Initialized connector for %s:%d", host.c_str(), port);
        m_canConnect = true;
    }

    bool Connector::canConnect() const {
        return m_canConnect;
    }

    bool Connector::checkConnection() const {
        if (!m_canConnect) return false;
        return getOsInfo(nullptr) == ERR_Ok;
    }

    void Connector::setServer(std::string host, u16 port) {
        m_host = std::move(host);
        m_port = port;
        m_canConnect = true;
    }

    std::string Connector::getHostStr() {
        return m_host;
    }

    ERR Connector::getOsInfo(OSInfo *res) const {
        auto req = proto::Request(proto::REQ_OS_INFO);
        ERR err = ERR_Ok;
        proto::Response *resp = exec(&req, &err);

        if (err != ERR_Ok) {
            return err;
        }

        if (res) {
            *res = reinterpret_cast<proto::OsInfoResponse *>(resp)->info;
        }

        return err;
    }

    proto::Response *Connector::exec(proto::Request *req, ERR *err) const {
        return connector::tcp::exec(req, err, m_host, m_port);
    }
}
