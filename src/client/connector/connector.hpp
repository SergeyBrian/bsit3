#ifndef CONNECTOR_HPP
#define CONNECTOR_HPP

#include <string>

#include "../../common/alias.hpp"
#include "../../common/data.hpp"
#include "../../common/errors.hpp"
#include "../../common/proto/request.hpp"
#include "../../common/proto/response.hpp"

namespace connector {
class Connector {
public:
    Connector(const std::string &host, u16 port);

    [[nodiscard]] bool canConnect() const;

    [[nodiscard]] bool checkConnection() const;

    void setServer(std::string host, u16 port);

    std::string getHostStr();

    ERR getOsInfo(OSInfo *res) const;

    ERR getTime(u64 *time, i8 *time_zone = nullptr) const;

    ERR getUptime(u64 *res) const;

    ERR getMemory(MemInfo *res) const;

    ERR getDrives(std::vector<DriveInfo> *res) const;

    ERR getRights(AccessRightsInfo *res, const std::wstring &str) const;

    ERR getOwner(OwnerInfo *res, const std::wstring &str) const;

private:
    std::string m_host;
    u16 m_port;
    bool m_canConnect;

    proto::Response *exec(proto::Request *req, ERR *err) const;
};
}  // namespace connector

#endif
