#ifndef BSIT_3_PACKABLE_HPP
#define BSIT_3_PACKABLE_HPP

#include <cstring>
#include <memory>

#include "../alias.hpp"
#include "../tcp_utils.hpp"

namespace proto {
struct Packable {
    virtual std::unique_ptr<const u8[]> pack(usize *size) const = 0;
};

class PackCtx {
public:
    PackCtx() {
        m_size = sizeof(m_size);
        m_tmp_buf_size = m_size;
        m_tmp_buf = std::make_unique<u8[]>(m_size);
    }

    PackCtx(const u8 *buf) {
        m_size = *reinterpret_cast<const usize *>(buf);
        m_tmp_buf = std::make_unique<u8[]>(m_size);
        std::memcpy(m_tmp_buf.get(), buf, m_size);
    }

    ~PackCtx() = default;

    template <typename T>
    void push(T val) {
        while (m_size + sizeof(val) >= m_tmp_buf_size) {
            usize old_size = m_tmp_buf_size;
            m_tmp_buf_size *= 2;
            auto tmp = std::make_unique<u8[]>(m_tmp_buf_size);
            std::memcpy(tmp.get(), m_tmp_buf.get(), old_size);
            m_tmp_buf = std::move(tmp);
        }
        *reinterpret_cast<T *>(m_tmp_buf.get() + m_size) =
            utils::hton_generic(val);
        m_size += sizeof(val);
    }

    template <typename T>
    void push(T *val, usize size) {
        u32 count = size / sizeof(*val);
        push(size);
        for (u32 i = 0; i < count; i++) {
            push(val[i]);
        }
    }

    std::unique_ptr<const u8[]> pack(usize *size) const {
        *size = m_size;
        auto res = std::make_unique<u8[]>(m_size);
        std::memcpy(res.get(), m_tmp_buf.get(), m_size);
        *reinterpret_cast<usize *>(res.get()) = m_size;

        return res;
    }

    template <typename T>
    T pop() {
        T res = utils::ntoh_generic(
            *reinterpret_cast<T *>(m_tmp_buf.get() + m_pop_offset));
        m_pop_offset += sizeof(T);
        return res;
    }

    template <typename T>
    std::unique_ptr<T[]> pop(usize *size) {
        *size = pop<usize>();
        auto ptr = reinterpret_cast<T *>(m_tmp_buf.get() + m_pop_offset);
        m_pop_offset += *size;
        auto res = std::make_unique<T[]>(*size);
        for (u64 i = 0; i < *size; i++) {
            res[i] = utils::ntoh_generic(ptr[i]);
        }
        return res;
    }

private:
    usize m_size = 0;
    usize m_tmp_buf_size = 0;
    usize m_pop_offset = sizeof(m_size);
    std::unique_ptr<u8[]> m_tmp_buf{};
};
}  // namespace proto

#endif
