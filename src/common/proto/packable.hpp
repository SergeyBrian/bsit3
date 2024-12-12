#ifndef BSIT_3_PACKABLE_HPP
#define BSIT_3_PACKABLE_HPP

#include <cstring>

#include "../alias.hpp"
#include "../tcp_utils.hpp"

namespace proto {
    struct Packable {
        virtual const u8 *pack(size_t *size) const = 0;
    };

    class PackCtx {
    public:
        PackCtx() {
            m_size = sizeof(m_size);
            m_tmp_buf_size = m_size;
            m_tmp_buf = new u8[m_size];
        }

        PackCtx(const u8 *buf) {
            m_size = *reinterpret_cast<const size_t *>(buf);
            m_tmp_buf = new u8[m_size];
            std::memcpy(m_tmp_buf, buf, m_size);
        }

        ~PackCtx() {
            delete[] m_tmp_buf;
        }

        template<typename T>
        void push(T val) {
            while (m_size + sizeof(val) >= m_tmp_buf_size) {
                size_t old_size = m_tmp_buf_size;
                m_tmp_buf_size *= 2;
                auto tmp = new u8[m_tmp_buf_size];
                std::memcpy(tmp, m_tmp_buf, old_size);
                delete[] m_tmp_buf;
                m_tmp_buf = tmp;
            }
            *reinterpret_cast<T *>(m_tmp_buf + m_size) = utils::hton_generic(val);
            m_size += sizeof(val);
        }

        template<typename T>
        void push(T *val, size_t size) {
            u32 count = size / sizeof(val);
            push(size);
            for (u32 i = 0; i < count; i++) {
                push(val[i]);
            }
        }

        u8 *pack(size_t *size) const {
            *size = m_size;
            auto res = new u8[m_size];
            std::memcpy(res, m_tmp_buf, m_size);
            *reinterpret_cast<size_t *>(res) = m_size;

            return res;
        }

        template<typename T>
        T pop() {
            T res = utils::ntoh_generic(*reinterpret_cast<T*>(m_tmp_buf + m_pop_offset));
            m_pop_offset += sizeof(T);
            return res;
        }

        template<typename T>
        T *pop(size_t *size) {
            *size = *m_tmp_buf;
            m_pop_offset += sizeof(size);
            auto ptr = reinterpret_cast<T *>(m_tmp_buf);
            m_pop_offset += *size;
            auto res = new T[*size];
            for (u64 i = 0; i < *size; i++) {
                res[i] = utils::ntoh_generic(ptr[i]);
            }
            return res;
        }

    private:
        size_t m_size = 0;
        size_t m_tmp_buf_size = 0;
        size_t m_pop_offset = sizeof(m_size);
        u8 *m_tmp_buf = nullptr;
    };
}

#endif
