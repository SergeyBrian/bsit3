#ifndef BSIT_3_TCP_UTILS_HPP
#define BSIT_3_TCP_UTILS_HPP

#include <bit>

#include "alias.hpp"

namespace utils {

template <typename T>
constexpr T byteswap_generic(T value) {
    static_assert(std::is_integral_v<T>,
                  "byteswap_generic requires an integral type");
    static_assert(
        std::is_unsigned_v<T>,
        "byteswap_generic requires an unsigned type for correct behavior");

    if constexpr (sizeof(T) == sizeof(u16)) {
        u16 v = static_cast<u16>(value);
        v = static_cast<u16>((v << 8) | (v >> 8));
        return static_cast<T>(v);
    } else if constexpr (sizeof(T) == sizeof(u32)) {
        u32 v = static_cast<u32>(value);
        v = ((v & 0x000000FFu) << 24) | ((v & 0x0000FF00u) << 8) |
            ((v & 0x00FF0000u) >> 8) | ((v & 0xFF000000u) >> 24);
        return static_cast<T>(v);
    } else if constexpr (sizeof(T) == sizeof(u64)) {
        u64 v = static_cast<u64>(value);
        v = ((v & 0x00000000000000FFULL) << 56) |
            ((v & 0x000000000000FF00ULL) << 40) |
            ((v & 0x0000000000FF0000ULL) << 24) |
            ((v & 0x00000000FF000000ULL) << 8) |
            ((v & 0x000000FF00000000ULL) >> 8) |
            ((v & 0x0000FF0000000000ULL) >> 24) |
            ((v & 0x00FF000000000000ULL) >> 40) |
            ((v & 0xFF00000000000000ULL) >> 56);
        return static_cast<T>(v);
    } else {
        return value;
    }
}

template <typename T>
constexpr T ntoh_generic(T value) {
    if constexpr (std::endian::native == std::endian::little) {
        return static_cast<T>(
            byteswap_generic(static_cast<std::make_unsigned_t<T>>(value)));
    } else {
        return value;
    }
}

template <typename T>
constexpr T hton_generic(T value) {
    if constexpr (std::endian::native == std::endian::little) {
        return static_cast<T>(
            byteswap_generic(static_cast<std::make_unsigned_t<T>>(value)));
    } else {
        return value;
    }
}
void dump_memory(const void *ptr, usize size);
}  // namespace utils

#endif
