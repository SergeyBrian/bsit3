#ifndef ERRORS_HPP
#define ERRORS_HPP

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
        "Can't Connect to server",
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
        default:
            return ERR_Unknown;
    }
}

#endif