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
        [[nodiscard]] size_t size() const;
        [[nodiscard]] const u8 *buf() const;

        static bool ValidateBuff(const u8 *buf, size_t size);
    private:
        explicit Message(Packable *p, MessageType type);
        MessageType m_type;
        const u8 *m_buf{};
        size_t m_size{};
    };

}

#endif
