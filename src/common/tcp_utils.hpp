#ifndef BSIT_3_TCP_UTILS_HPP
#define BSIT_3_TCP_UTILS_HPP

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>

#include "alias.hpp"

namespace utils {
    template<typename T>
    T ntoh_generic(T value) {
        if constexpr (sizeof(T) == sizeof(uint16_t)) {
            return ntohs(static_cast<uint16_t>(value));
        } else if constexpr (sizeof(T) == sizeof(uint32_t)) {
            return ntohl(static_cast<uint32_t>(value));
        } else if constexpr (sizeof(T) == sizeof(uint64_t)) {
            uint32_t high_part = ntohl(static_cast<uint32_t>(value >> 32));
            uint32_t low_part = ntohl(static_cast<uint32_t>(value & 0xFFFFFFFF));
            return (static_cast<uint64_t>(low_part) << 32) | high_part;
        } else {
            return value;
        }
    }

    template<typename T>
    T hton_generic(T value) {
        if constexpr (sizeof(T) == sizeof(uint16_t)) {
            return htons(static_cast<uint16_t>(value));
        } else if constexpr (sizeof(T) == sizeof(uint32_t)) {
            return htonl(static_cast<uint32_t>(value));
        } else if constexpr (sizeof(T) == sizeof(uint64_t)) {
            uint32_t high_part = htonl(static_cast<uint32_t>(value >> 32));
            uint32_t low_part = htonl(static_cast<uint32_t>(value & 0xFFFFFFFF));
            return (static_cast<uint64_t>(low_part) << 32) | high_part;
        } else {
            return value;
        }

    }
}

#endif
