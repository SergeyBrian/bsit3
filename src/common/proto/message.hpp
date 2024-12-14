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
};

class Message {
public:
    Message();
    explicit Message(const u8 *buf);

    explicit Message(Request *req);
    explicit Message(Response *resp);
    ~Message();

    [[nodiscard]] MessageType type() const;
    [[nodiscard]] usize size() const;
    [[nodiscard]] const u8 *buf() const;

    static bool ValidateBuff(const u8 *buf, usize size);

private:
    explicit Message(Packable *p, MessageType type);
    MessageType m_type;
    const u8 *m_buf = nullptr;
    usize m_size = 0;
};

}  // namespace proto

#endif
