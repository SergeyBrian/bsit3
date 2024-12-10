#include "os_utils.hpp"

#include <chrono>
#include <iostream>

#include <windows.h>
#include <sddl.h>
#include <AclAPI.h>


std::string GetLastErrorStdStr() {
    DWORD error = GetLastError();
    if (error) {
        LPVOID lpMsgBuf;
        DWORD bufLen = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                error,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR) &lpMsgBuf,
                0, NULL);
        if (bufLen) {
            LPCSTR lpMsgStr = (LPCSTR) lpMsgBuf;
            std::string result(lpMsgStr, lpMsgStr + bufLen);

            LocalFree(lpMsgBuf);

            return result;
        }
    }
    return std::string();
}

namespace os_utils {
    OSType get_type() {
        HMODULE hModule = GetModuleHandle(TEXT("kernel32.dll"));
        if (!hModule) return OS_UNKNOWN;

        typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS2)(HANDLE, USHORT*, USHORT*);
        auto IsWow64Process2 = (LPFN_ISWOW64PROCESS2)GetProcAddress(hModule, "IsWow64Process2");

        if (IsWow64Process2) {
            USHORT processMachine = 0;
            USHORT nativeMachine = 0;
            if (IsWow64Process2(GetCurrentProcess(), &processMachine, &nativeMachine)) {
                switch (nativeMachine) {
                    case IMAGE_FILE_MACHINE_I386: return OS_WIN32;
                    case IMAGE_FILE_MACHINE_AMD64: return OS_WIN64;
                    case IMAGE_FILE_MACHINE_ARM:   return OS_WINARM;
                    case IMAGE_FILE_MACHINE_ARM64: return OS_WINARM64;
                    default: return OS_UNKNOWN;
                }
            }
        } else {
            BOOL isWow64 = FALSE;
            if (IsWow64Process(GetCurrentProcess(), &isWow64)) {
                SYSTEM_INFO sysInfo;
                GetNativeSystemInfo(&sysInfo);

                switch (sysInfo.wProcessorArchitecture) {
                    case PROCESSOR_ARCHITECTURE_INTEL: return OS_WIN32;
                    case PROCESSOR_ARCHITECTURE_AMD64: return OS_WIN64;
                    case PROCESSOR_ARCHITECTURE_ARM:   return OS_WINARM;
                    case PROCESSOR_ARCHITECTURE_ARM64: return OS_WINARM64;
                    default: return OS_UNKNOWN;
                }
            }
        }

        return OS_UNKNOWN;
    }

    OSVersion get_version() {
        OSVersion version = {0, 0};

        typedef LONG (WINAPI *RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
        if (HMODULE hMod = GetModuleHandleW(L"ntdll.dll")) {
            if (auto pRtlGetVersion = reinterpret_cast<RtlGetVersionPtr>(GetProcAddress(hMod, "RtlGetVersion"))) {
                RTL_OSVERSIONINFOW rovi = {0};
                rovi.dwOSVersionInfoSize = sizeof(rovi);
                if (pRtlGetVersion(&rovi) == 0) {
                    version.major = rovi.dwMajorVersion;
                    version.minor = rovi.dwMinorVersion;
                }
            }
        }
        return version;
    }

    u64 get_uptime_ms() {
        return static_cast<u64>(GetTickCount64());
    }

    MemInfo get_meminfo() {
        MemInfo memInfo = {0};
        MEMORYSTATUSEX status;
        status.dwLength = sizeof(status);
        if (!GlobalMemoryStatusEx(&status)) {
            std::cerr << "Can't get memory info" << std::endl;
            return memInfo;
        }

        memInfo.total_bytes = status.ullTotalPhys;
        memInfo.free_bytes = status.ullAvailPhys;
        return memInfo;
    }

    std::vector<DriveInfo> get_drives() {
        std::vector<DriveInfo> drives;

        DWORD driveMask = GetLogicalDrives();
        if (driveMask == 0) {
            std::cerr << "GetLogicalDrives() failed with error " << GetLastError() << std::endl;
            return drives;
        }

        for (char drive = 'A'; drive <= 'Z'; ++drive) {
            if (driveMask & (1 << (drive - 'A'))) {
                std::string driveName = std::string(1, drive) + ":\\";
                UINT type = GetDriveTypeA(driveName.c_str());

                DriveType driveType = DRIVE_TYPE_UNKNOWN;
                switch (type) {
                    case DRIVE_FIXED:
                        driveType = DRIVE_TYPE_LOCAL;
                        break;
                    case DRIVE_REMOTE:
                        driveType = DRIVE_TYPE_NET;
                        break;
                    case DRIVE_REMOVABLE:
                    case DRIVE_CDROM:
                        driveType = DRIVE_TYPE_REMOVABLE;
                        break;
                    case DRIVE_RAMDISK:
                        driveType = DRIVE_TYPE_FS;
                        break;
                    default:
                        driveType = DRIVE_TYPE_UNKNOWN;
                }

                ULARGE_INTEGER freeBytesAvailable, totalNumberOfBytes, totalNumberOfFreeBytes;
                BOOL success = GetDiskFreeSpaceExA(
                        driveName.c_str(),
                        &freeBytesAvailable,
                        &totalNumberOfBytes,
                        &totalNumberOfFreeBytes
                );

                uint64_t freeBytes = 0;
                if (success) {
                    freeBytes = static_cast<uint64_t>(totalNumberOfFreeBytes.QuadPart);
                } else {
                    std::cerr << "GetDiskFreeSpaceExA() failed for " << driveName
                              << " with error " << GetLastError() << std::endl;
                }

                drives.emplace_back(DriveInfo{driveType, driveName, freeBytes});
            }
        }

        return drives;
    }

    AccessRightsInfo get_access_info(const std::wstring &path) {
        AccessRightsInfo rightsInfo = {};

        PSECURITY_DESCRIPTOR securityDescriptor = nullptr;
        PACL dacl = nullptr;

        if (GetNamedSecurityInfoW(
                path.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION,
                nullptr, nullptr, &dacl, nullptr, &securityDescriptor) != ERROR_SUCCESS) {
            if (GetNamedSecurityInfoW(
                    path.c_str(), SE_REGISTRY_KEY, DACL_SECURITY_INFORMATION,
                    nullptr, nullptr, &dacl, nullptr, &securityDescriptor) != ERROR_SUCCESS) {
                return rightsInfo;
            }
        }

        for (DWORD i = 0; i < dacl->AceCount; i++) {
            PACE_HEADER aceHeader = nullptr;
            if (!GetAce(dacl, i, reinterpret_cast<void **>(&aceHeader))) {
                continue;
            }

            AccessControlEntry entry = {};
            PSID sid = nullptr;

            if (aceHeader->AceType == ACCESS_ALLOWED_ACE_TYPE) {
                auto ace = reinterpret_cast<ACCESS_ALLOWED_ACE *>(aceHeader);
                sid = &ace->SidStart;
                entry.aceType = ACE_TYPE_ALLOWED;
                entry.accessMask = ace->Mask;
            } else if (aceHeader->AceType == ACCESS_DENIED_ACE_TYPE) {
                auto ace = reinterpret_cast<ACCESS_DENIED_ACE *>(aceHeader);
                sid = &ace->SidStart;
                entry.aceType = ACE_TYPE_DENIED;
                entry.accessMask = ace->Mask;
            } else {
                entry.aceType = ACE_TYPE_OTHER;
                continue;
            }

            DWORD sidLength = GetLengthSid(sid);
            memcpy(entry.sid.data(), sid, sidLength);

            if (aceHeader->AceFlags & OBJECT_INHERIT_ACE) {
                entry.scope = SCOPE_OBJECT;
            } else if (aceHeader->AceFlags & CONTAINER_INHERIT_ACE) {
                entry.scope = SCOPE_CONTAINER;
            } else {
                entry.scope = SCOPE_DIRECT;
            }

            rightsInfo.entries.push_back(entry);
        }

        LocalFree(securityDescriptor);

        return rightsInfo;
    }

    OwnerInfo get_owner_info(const std::wstring &path) {
        OwnerInfo ownerInfo = {};
        PSECURITY_DESCRIPTOR securityDescriptor = nullptr;
        PSID ownerSid = nullptr;

        if (GetNamedSecurityInfoW(
                path.c_str(), SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION,
                &ownerSid, nullptr, nullptr, nullptr, &securityDescriptor) != ERROR_SUCCESS) {
            if (GetNamedSecurityInfoW(
                    path.c_str(), SE_REGISTRY_KEY, OWNER_SECURITY_INFORMATION,
                    &ownerSid, nullptr, nullptr, nullptr, &securityDescriptor) != ERROR_SUCCESS) {
                return ownerInfo;
            }
        }

        if (!IsValidSid(ownerSid)) {
            LocalFree(securityDescriptor);
            return ownerInfo;
        }

        char name[256], domain[256];
        DWORD nameSize = sizeof(name);
        DWORD domainSize = sizeof(domain);
        SID_NAME_USE sidType;

        if (LookupAccountSidA(nullptr, ownerSid, name, &nameSize, domain, &domainSize, &sidType)) {
            ownerInfo.ownerName = name;
            ownerInfo.ownerDomain = domain;
        }

        LocalFree(securityDescriptor);

        return ownerInfo;
    }
}
