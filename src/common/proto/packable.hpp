#ifndef BSIT_3_PACKABLE_HPP
#define BSIT_3_PACKABLE_HPP

#include "../alias.hpp"

namespace proto {
    struct Packable {
        virtual const u8 *pack(size_t *size) const = 0;
    };
}

#endif
