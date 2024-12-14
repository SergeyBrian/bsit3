#include "message.hpp"

#include <cassert>
#include <cstring>

#include "../logging.hpp"
#include "../tcp_utils.hpp"

namespace proto {
Message::Message(Packable *p, MessageType type) {
    m_size = 0;
    m_type = type;

    const u8 *content_buf = p->pack(&m_size);
    size_t content_size = m_size;

    m_size += sizeof(m_size);
    m_size += sizeof(m_type);

    m_buf = new u8[m_size];
    auto buf = const_cast<u8 *>(m_buf);

    *reinterpret_cast<size_t *>(buf) = m_size;
    buf += sizeof(m_size);
    *buf = m_type;
    buf += sizeof(m_type);
    std::memcpy(buf, content_buf, content_size);
}

Message::Message(Request *req) : Message(req, MESSAGE_REQUEST) {}

Message::Message(Response *resp) : Message(resp, MESSAGE_RESPONSE) {}

size_t Message::size() const { return m_size; }

MessageType Message::type() const { return m_type; }

const u8 *Message::buf() const { return m_buf; }

Message::Message(const u8 *buf) {
    m_size = utils::hton_generic(*reinterpret_cast<const size_t *>(buf));
    INFO("Received Message of size %lu", m_size);
    utils::dump_memory(buf, MAX_MSG_SIZE);
    assert(m_size <= MAX_MSG_SIZE);
    buf += sizeof(m_size);
    m_type = static_cast<MessageType>(*buf);
    buf += sizeof(m_type);
    m_buf = new u8[m_size];
    std::memcpy(const_cast<u8 *>(m_buf), buf, m_size);
}

bool Message::ValidateBuff(const u8 *buf, size_t size) {
    auto msg_size = *reinterpret_cast<const size_t *>(buf);
    return msg_size == size;
}

Message::~Message() { delete[] m_buf; }

Message::Message() = default;
}  // namespace proto
