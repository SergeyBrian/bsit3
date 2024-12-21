#ifndef CONNECTOR_HPP
#define CONNECTOR_HPP

#include <string>

#include "../../common/alias.hpp"
#include "../../common/data.hpp"
#include "../../common/errors.hpp"
#include "../../common/proto/request.hpp"
#include "../../common/proto/response.hpp"
#include "context.hpp"

namespace connector {
class Connector {
public:
    Connector(u32 cid, const std::string &host, u16 port);

    [[nodiscard]] bool canConnect() const;

    bool checkConnection();

    void setServer(std::string host, u16 port);

    std::string getHostStr();

    ERR getOsInfo(OSInfo *res);

    ERR getTime(u64 *time, i8 *time_zone = nullptr);

    ERR getUptime(u64 *res);

    ERR getMemory(MemInfo *res);

    ERR getDrives(std::vector<DriveInfo> *res);

    ERR getRights(AccessRightsInfo *res, const std::wstring &str);

    ERR getOwner(OwnerInfo *res, const std::wstring &str);

    void disconnect();

    ERR reconnect();

private:
    std::string m_host;
    u16 m_port;
    bool m_canConnect;
    tcp::Context *m_ctx = nullptr;
    u32 m_id;

    proto::Response *exec(proto::Request *req, ERR *err);
};
}  // namespace connector

#endif
