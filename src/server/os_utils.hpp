#ifndef OS_UTILS_H
#define OS_UTILS_H

#include <vector>

#include "../common/data.hpp"

namespace os_utils {
OSType get_type();
OSVersion get_version();
u64 get_uptime_ms();
u64 get_time_ms();
i8 get_timezone_hours();
MemInfo get_meminfo();
std::vector<DriveInfo> get_drives();
AccessRightsInfo get_access_info(const std::wstring &path);
OwnerInfo get_owner_info(const std::wstring &path);
}  // namespace os_utils

#endif
