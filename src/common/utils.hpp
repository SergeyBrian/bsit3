#ifndef UTILS_H
#define UTILS_H

#include <string>

#include "alias.hpp"
#include "data.hpp"

namespace utils {
std::string format_milliseconds(u64 ms);

std::string format_bytes(u64 bytes);

void print_access_rights(const AccessRightsInfo &rightsInfo);

std::string format_time(u64 ms);

std::string to_string(const std::wstring &wstr);
}  // namespace utils

#endif
