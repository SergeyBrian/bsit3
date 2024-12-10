#include "test.hpp"

#include <iostream>
#include <windows.h>
#include <locale>
#include <codecvt>

#include "os_utils.hpp"
#include "../common/utils.hpp"

namespace test {
    void PrintTestOutput(char **argv) {
        int len = MultiByteToWideChar(CP_UTF8, 0, argv[0], -1, nullptr, 0);
        std::wstring programPath(len, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, argv[0], -1, &programPath[0], len);

        programPath.resize(len - 1);

        std::locale::global(std::locale("Russian_Russia.1251"));
        SetConsoleOutputCP(1251);
        SetConsoleCP(1251);
        OSVersion version = os_utils::get_version();
        std::cout << OSTypeName[os_utils::get_type()] << " " << version.major << "." << version.minor << std::endl;
        std::cout << "Time since start: " << utils::format_milliseconds(os_utils::get_uptime_ms()) << std::endl;
        MemInfo mem = os_utils::get_meminfo();
        std::cout << "Total memory: " << utils::format_bytes(mem.total_bytes) << std::endl;
        std::cout << "Free memory: " << utils::format_bytes(mem.free_bytes) << std::endl;
        auto drives = os_utils::get_drives();
        std::cout << "Mounted drives:\n";
        for (const auto &[type, name, free_bytes]: drives) {
            std::cout << "\t" << name << " [" << DriveTypeName[type] << "] Free space: "
                      << utils::format_bytes(free_bytes)
                      << std::endl;
        }
        AccessRightsInfo rightsInfo = os_utils::get_access_info(programPath);
        std::cout << "Test access rights:\n";
        utils::print_access_rights(rightsInfo);
        OwnerInfo ownerInfo = os_utils::get_owner_info(programPath);
        std::cout << "Test owner info:\n\t";
        std::cout << ownerInfo.ownerDomain << "\\" << ownerInfo.ownerName << "\n";
        std::wstring testRegKey = L"CURRENT_USER\\Software\\Chromium\\BLBeacon";
        std::cout << "Test registry key access rights:\n";
        rightsInfo = os_utils::get_access_info(testRegKey);
        utils::print_access_rights(rightsInfo);
        std::cout << "Test registry key owner info:\n";
        ownerInfo = os_utils::get_owner_info(testRegKey);
        std::cout << ownerInfo.ownerDomain << "\\" << ownerInfo.ownerName << "\n";
    }
}
