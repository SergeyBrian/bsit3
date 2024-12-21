#include "connector.hpp"

#include <utility>

#include "../../common/logging.hpp"
#include "../../common/proto/encryption/encryption.hpp"
#include "../../common/proto/proto.hpp"

namespace connector {
Connector::Connector(u32 cid, const std::string &host, u16 port) {
    m_id = cid;
    this->m_host = host;
    this->m_port = port;

    if (host.empty() && port == 0) {
        m_canConnect = false;
        OKAY("Initialized connector without initial host and port");
        return;
    }
    OKAY("Initialized connector for %s:%d", host.c_str(), port);
    m_canConnect = true;
    proto::encryption::init();
    proto::encryption::g_instance->CreateAsymmetricKey();
}

bool Connector::canConnect() const { return m_canConnect; }

bool Connector::checkConnection() {
    if (!m_canConnect) return false;
    bool res = getOsInfo(nullptr) == ERR_Ok;
    m_canConnect = res;
    return res;
}

void Connector::setServer(std::string host, u16 port) {
    m_host = std::move(host);
    m_port = port;
    m_canConnect = true;
}

std::string Connector::getHostStr() { return m_host; }

proto::Response *Connector::exec(proto::Request *req, ERR *err) {
    *err = ERR_Ok;
    if (!m_ctx || m_ctx->Expired()) {
        INFO("Reconnecting to the server...");
        *err = reconnect();
        if (*err != ERR_Ok) {
            INFO("Error connecting to server: %s", errorText[*err]);
            return nullptr;
        }
    }

    int res = 0;

    auto msg = proto::Message(req, proto::MESSAGE_ENCRYPTION_SYMMETRIC, m_id);
    *err = m_ctx->Send(&msg);
    if (*err != ERR_Ok) {
        return nullptr;
    }
    proto::Message resp_msg = m_ctx->Receive(err);
    if (*err != ERR_Ok) {
        return nullptr;
    }
    proto::Response *resp = proto::ParseResponse(&resp_msg, err);

    return resp;
}

ERR Connector::getOsInfo(OSInfo *res) {
    auto req = proto::Request(proto::REQ_OS_INFO);
    ERR err = ERR_Ok;
    proto::Response *resp = exec(&req, &err);

    if (err != ERR_Ok) {
        return err;
    }

    if (res) {
        *res = reinterpret_cast<proto::OsInfoResponse *>(resp)->info;
    }
    delete resp;

    return err;
}

ERR Connector::getTime(u64 *res, i8 *time_zone) {
    auto req = proto::Request(proto::REQ_TIME);
    ERR err = ERR_Ok;
    proto::Response *resp = exec(&req, &err);

    if (err != ERR_Ok) {
        return err;
    }

    if (res) {
        *res = reinterpret_cast<proto::TimeResponse *>(resp)->time_ms;
    }

    if (time_zone) {
        *time_zone = reinterpret_cast<proto::TimeResponse *>(resp)->time_zone;
    }
    delete resp;

    return err;
}

ERR Connector::getUptime(u64 *res) {
    auto req = proto::Request(proto::REQ_UPTIME);
    ERR err = ERR_Ok;
    proto::Response *resp = exec(&req, &err);

    if (err != ERR_Ok) {
        return err;
    }

    if (res) {
        *res = reinterpret_cast<proto::TimeResponse *>(resp)->time_ms;
    }
    delete resp;

    return err;
}

ERR Connector::getMemory(MemInfo *res) {
    auto req = proto::Request(proto::REQ_MEMORY);
    ERR err = ERR_Ok;
    proto::Response *resp = exec(&req, &err);

    if (err != ERR_Ok) {
        return err;
    }

    if (res) {
        *res = reinterpret_cast<proto::MemoryResponse *>(resp)->mem_info;
    }
    delete resp;

    return err;
}

ERR Connector::getDrives(std::vector<DriveInfo> *res) {
    auto req = proto::Request(proto::REQ_DRIVES);
    ERR err = ERR_Ok;
    proto::Response *resp = exec(&req, &err);

    if (err != ERR_Ok) {
        return err;
    }

    if (res) {
        *res = reinterpret_cast<proto::DrivesResponse *>(resp)->drives;
    }
    delete resp;

    return err;
}

ERR Connector::getRights(AccessRightsInfo *res, const std::wstring &str) {
    auto req = proto::Request(proto::REQ_RIGHTS, str);
    ERR err = ERR_Ok;
    proto::Response *resp = exec(&req, &err);

    if (err != ERR_Ok) {
        return err;
    }

    if (res) {
        *res = reinterpret_cast<proto::RightsResponse *>(resp)->rights_info;
    }
    delete resp;

    return err;
}

ERR Connector::getOwner(OwnerInfo *res, const std::wstring &str) {
    auto req = proto::Request(proto::REQ_OWNER, str);
    ERR err = ERR_Ok;
    proto::Response *resp = exec(&req, &err);

    if (err != ERR_Ok) {
        return err;
    }

    if (res) {
        *res = reinterpret_cast<proto::OwnerResponse *>(resp)->info;
    }
    delete resp;

    return err;
}
ERR Connector::reconnect() {
    INFO("Removing old context");
    delete m_ctx;
    OKAY("Done.");
    INFO("Creating new context");
    m_ctx = new tcp::Context(m_id);
    OKAY("Done");
    ERR err = m_ctx->Connect(m_host, m_port);
    if (err != ERR_Ok) {
        return err;
    }

    DWORD size;
    auto buf = proto::encryption::g_instance->ExportPublicKey(&size);
    proto::Message msg(proto::MESSAGE_KEY_REQUEST, buf, size,
                       proto::MESSAGE_ENCRYPTION_NONE);
    INFO("Requesting key...");
    err = m_ctx->Send(&msg);
    if (err != ERR_Ok) {
        return err;
    }
    OKAY("Key request sent");

    INFO("Receiving key response...");
    proto::Message resp_msg = m_ctx->Receive(&err);
    if (err != ERR_Ok) {
        return err;
    }
    OKAY("Key received");

    proto::encryption::g_instance->ImportSymmetricKey(m_id, resp_msg.buf(),
                                                      resp_msg.size());

    return err;
}
void Connector::disconnect() {
    INFO("Disconnecting");
    delete m_ctx;
    m_ctx = nullptr;
}
}  // namespace connector
