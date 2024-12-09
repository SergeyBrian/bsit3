#include "utils.hpp"
#include "data.hpp"

#include <sstream>
#include <iostream>
#include <bitset>
#include <iomanip>
#include <windows.h>
#include <sddl.h>

namespace utils {
    std::string format_unit(const u64 val, const std::string &singular, const std::string &plural) {
        return std::to_string(val) + " " + (val == 1 ? singular : plural);
    }

    std::string format_milliseconds(const u64 ms) {
        const u64 total_seconds = ms / 1000;

        const u64 days = total_seconds / (24 * 3600);
        u64 remainder = total_seconds % (24 * 3600);

        const u64 hours = remainder / 3600;
        remainder %= 3600;

        const u64 minutes = remainder / 60;
        const u64 seconds = remainder % 60;

        std::ostringstream oss;
        if (days > 0) oss << format_unit(days, "day", "days") << " ";
        if (hours > 0) oss << format_unit(hours, "hour", "hours") << " ";
        if (minutes > 0) oss << format_unit(minutes, "minute", "minutes") << " ";
        oss << format_unit(seconds, "second", "seconds");

        return oss.str();
    }

    std::string format_bytes(u64 bytes) {
        const char *sizes[] = {"B", "KB", "MB", "GB", "TB"};
        int order = 0;
        auto dblBytes = static_cast<double>(bytes);
        while (dblBytes >= 1024 && order < 4) {
            order++;
            dblBytes /= 1024;
        }

        char buffer[20];
        snprintf(buffer, sizeof(buffer), "%.2f %s", dblBytes, sizes[order]);
        return {buffer};
    }

    void print_access_rights(const AccessRightsInfo &rightsInfo) {
        for (const auto& entry : rightsInfo.entries) {
            LPSTR sidString = nullptr;
            if (ConvertSidToStringSidA(reinterpret_cast<PSID>(const_cast<unsigned char *>(entry.sid.data())), &sidString)) {
                std::cout << "SID: " << sidString << "\n";
                LocalFree(sidString);
            } else {
                std::cout << "SID: (failed to convert SID to string format)\n";
            }

            std::cout << "ACE Type: ";
            switch (entry.aceType) {
                case ACE_TYPE_ALLOWED:
                    std::cout << "Allowed";
                    break;
                case ACE_TYPE_DENIED:
                    std::cout << "Denied";
                    break;
                default:
                    std::cout << "Other";
            }
            std::cout << "\n";

            std::cout << "Scope: ";
            switch (entry.scope) {
                case SCOPE_DIRECT:
                    std::cout << "Direct";
                    break;
                case SCOPE_OBJECT:
                    std::cout << "Object";
                    break;
                case SCOPE_CONTAINER:
                    std::cout << "Container";
                    break;
            }
            std::cout << "\n";

            std::cout << "Access Mask: 0x" << std::hex << entry.accessMask << std::dec << "\n";
            std::cout << "Set Bit Names:\n";

            if (entry.accessMask & DELETE) {
                std::cout << "\t+ DELETE\n";
            }
            if (entry.accessMask & READ_CONTROL) {
                std::cout << "\t+ READ_CONTROL\n";
            }
            if (entry.accessMask & WRITE_DAC) {
                std::cout << "\t+ WRITE_DAC\n";
            }
            if (entry.accessMask & WRITE_OWNER) {
                std::cout << "\t+ WRITE_OWNER\n";
            }
            if (entry.accessMask & SYNCHRONIZE) {
                std::cout << "\t+ SYNCHRONIZE\n";
            }
            if (entry.accessMask & STANDARD_RIGHTS_REQUIRED) {
                std::cout << "\t+ STANDARD_RIGHTS_REQUIRED\n";
            }
            if (entry.accessMask & STANDARD_RIGHTS_READ) {
                std::cout << "\t+ STANDARD_RIGHTS_READ\n";
            }
            if (entry.accessMask & STANDARD_RIGHTS_WRITE) {
                std::cout << "\t+ STANDARD_RIGHTS_WRITE\n";
            }
            if (entry.accessMask & STANDARD_RIGHTS_EXECUTE) {
                std::cout << "\t+ STANDARD_RIGHTS_EXECUTE\n";
            }
            if (entry.accessMask & STANDARD_RIGHTS_ALL) {
                std::cout << "\t+ STANDARD_RIGHTS_ALL\n";
            }
            if (entry.accessMask & SPECIFIC_RIGHTS_ALL) {
                std::cout << "\t+ SPECIFIC_RIGHTS_ALL\n";
            }

            std::cout << "\n";
        }
    }
}
