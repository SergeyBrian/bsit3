#include "tcp_utils.hpp"

#include <iomanip>
#include <iostream>

namespace utils {

void dump_memory(const void *ptr, usize size) {
    const uint8_t *bytePtr = static_cast<const uint8_t *>(ptr);
    const usize bytesPerLine = 16;

    for (usize i = 0; i < size; i += bytesPerLine) {
        std::cout << std::setw(8) << std::setfill('0') << std::hex
                  << reinterpret_cast<uintptr_t>(bytePtr + i) << ": ";

        for (usize j = 0; j < bytesPerLine; ++j) {
            if (i + j < size) {
                std::cout << std::setw(2) << static_cast<int>(bytePtr[i + j])
                          << " ";
            } else {
                std::cout << "   ";
            }
        }

        std::cout << " ";

        for (usize j = 0; j < bytesPerLine; ++j) {
            if (i + j < size) {
                uint8_t byte = bytePtr[i + j];
                if (std::isprint(byte)) {
                    std::cout << static_cast<char>(byte);
                } else {
                    std::cout << '.';
                }
            } else {
                std::cout << ' ';
            }
        }

        std::cout << std::dec << std::endl;
    }
}
}  // namespace utils
