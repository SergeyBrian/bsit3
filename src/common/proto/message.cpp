#include "message.hpp"

#include <cassert>
#include <cstring>

#include "../logging.hpp"
#include "../tcp_utils.hpp"
#include "encryption/encryption.hpp"

namespace proto {
Message::Message(Packable *p, MessageType type,
                 MessageEncryption encryption_method, u32 cid)
    : m_type(type), m_encryption(encryption_method), m_size(0) {
    auto content_buf = p->pack(&m_size);

    if (m_encryption == MESSAGE_ENCRYPTION_SYMMETRIC) {
        INFO("Encrypting message using symmetric method");
        DWORD encrypted_size;
        auto encrypted = encryption::g_instance->Encrypt(
            cid, content_buf.get(), m_size, &encrypted_size);
        content_buf = std::move(encrypted);
        m_size = encrypted_size;
    }

    usize content_size = m_size;

    m_size += sizeof(m_size);
    m_size += sizeof(m_type);
    m_size += sizeof(m_encryption);

    m_buf = std::make_unique<const u8[]>(m_size);
    auto buf = const_cast<u8 *>(m_buf.get());

    *reinterpret_cast<usize *>(buf) = utils::ntoh_generic(m_size);
    buf += sizeof(m_size);
    *buf = m_type;
    buf += sizeof(m_type);
    *buf = m_encryption;
    buf += sizeof(m_encryption);
    std::memcpy(buf, content_buf.get(), content_size);
}

Message::Message(MessageType type, const u8 *buf, usize size,
                 MessageEncryption encryption_method)
    : m_type(type), m_encryption(encryption_method), m_size(size) {
    m_size += sizeof(m_size);
    m_size += sizeof(m_type);
    m_size += sizeof(m_encryption);
    m_buf = std::make_unique<const u8[]>(m_size);
    auto tmp = const_cast<u8 *>(m_buf.get());

    *reinterpret_cast<usize *>(tmp) = utils::ntoh_generic(m_size);
    tmp += sizeof(m_size);
    *tmp = m_type;
    tmp += sizeof(m_type);
    *tmp = m_encryption;
    tmp += sizeof(m_encryption);
    std::memcpy(tmp, buf, size);
}

Message::Message(Request *req, MessageEncryption encryption_method, u32 cid)
    : Message(req, MESSAGE_REQUEST, encryption_method, cid) {}

Message::Message(Response *resp, MessageEncryption encryption_method, u32 cid)
    : Message(resp, MESSAGE_RESPONSE, encryption_method, cid) {}

usize Message::size() const { return m_size; }

MessageType Message::type() const { return m_type; }

const u8 *Message::buf() const { return m_buf.get(); }

Message::Message(u32 cid, const u8 *buf) {
    m_size = utils::ntoh_generic(*reinterpret_cast<const usize *>(buf));
    INFO("Received Message of size %llu", m_size);
    utils::dump_memory(buf, MIN(m_size, MAX_MSG_SIZE));
    assert(m_size <= MAX_MSG_SIZE);
    buf += sizeof(m_size);
    m_type = static_cast<MessageType>(*buf);
    buf += sizeof(m_type);
    m_encryption = static_cast<MessageEncryption>(*buf);
    buf += sizeof(m_encryption);

    m_buf = std::make_unique<const u8[]>(m_size);
    std::unique_ptr<const u8[]> decrypted;
    if (m_encryption == MESSAGE_ENCRYPTION_SYMMETRIC) {
        DWORD content_size =
            m_size - sizeof(m_size) - sizeof(m_type) - sizeof(m_encryption);
        DWORD decrypted_size;
        decrypted = encryption::g_instance->Decrypt(
            cid, buf, content_size, &decrypted_size);
        buf = decrypted.get();
    }
    std::memcpy(const_cast<u8 *>(m_buf.get()), buf, m_size);
}

bool Message::ValidateBuff(const u8 *buf, usize size) {
    auto msg_size = utils::ntoh_generic(*reinterpret_cast<const usize *>(buf));
    return msg_size == size;
}

Message::~Message() = default;

Message::Message() = default;
}  // namespace proto
