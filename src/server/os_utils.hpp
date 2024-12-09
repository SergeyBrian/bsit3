#ifndef OS_UTILS_H
#define OS_UTILS_H

#include <vector>

#include "../common/data.hpp"

namespace os_utils {
    OSType get_type();
    OSVersion get_version();
    u64 get_uptime_ms();
    MemInfo get_meminfo();
    std::vector<DriveInfo> get_drives();
    AccessRightsInfo get_access_info(const std::wstring& path);
    OwnerInfo get_owner_info(const std::wstring &path);
}

#endif
