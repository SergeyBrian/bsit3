#ifndef ERRORS_HPP
#define ERRORS_HPP

#include "alias.hpp"
#include "logging.hpp"

enum ERR : u8 {
    ERR_Ok,
    ERR_Permission_denied,
    ERR_Unknown,
    ERR_Connect,
    ERR_Invalid_Response,
    ERR_Count_
};

inline const char *errorText[ERR_Count_] = {
        "No error",
        "Permission denied",
        "Unknown error",
        "Connection refused by server",
        "Invalid response",
};

inline ERR winCodeToErr(int code) {
    switch (code) {
        case 0:
            return ERR_Ok;
        case 13:
        case 5:
        case 22:
            return ERR_Permission_denied;
        case 10061:
        case 10038:
            return ERR_Connect;
        default:
            WARN("Unknown error code: %d", code);
            return ERR_Unknown;
    }
}

#endif