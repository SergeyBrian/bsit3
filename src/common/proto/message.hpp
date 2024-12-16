#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "../alias.hpp"
#include "request.hpp"
#include "response.hpp"

#define MAX_MSG_SIZE 8192

namespace proto {

enum MessageType : u8 {
    MESSAGE_REQUEST,
    MESSAGE_RESPONSE,
    MESSAGE_KEY_REQUEST,
    MESSAGE_KEY_RESPONSE,
};

enum MessageEncryption : u8 {
    MESSAGE_ENCRYPTION_SYMMETRIC,
    MESSAGE_ENCRYPTION_ASYMMETRIC,
    MESSAGE_ENCRYPTION_NONE,
};

class Message {
public:
    Message();
    explicit Message(u32 cid, const u8 *buf);

    explicit Message(Request *req, MessageEncryption encryption_method,
                     u32 cid);
    explicit Message(Response *resp, MessageEncryption encryption_method,
                     u32 cid);
    Message(MessageType type, const u8 *buf, usize size,
            MessageEncryption encryption_method);

    ~Message();

    [[nodiscard]] MessageType type() const;
    [[nodiscard]] usize size() const;
    [[nodiscard]] const u8 *buf() const;

    static bool ValidateBuff(const u8 *buf, usize size);

private:
    explicit Message(Packable *p, MessageType type,
                     MessageEncryption encryption_method, u32 cid);
    MessageType m_type;
    MessageEncryption m_encryption;
    const u8 *m_buf = nullptr;
    usize m_size = 0;
};

}  // namespace proto

#endif
