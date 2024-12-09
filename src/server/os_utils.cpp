#include "os_utils.hpp"

#include <chrono>
#include <iostream>

#ifdef __linux__
#include <time.h>
#elif __APPLE__
#include <sys/sysctl.h>
#include <sys/time.h>
#elif _WIN32

#include <windows.h>
#include <sddl.h>
#include <AclAPI.h>

#endif


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
#ifdef _WIN32
#ifdef _WIN64
        return OS_WIN64;
#else
        return OS_WIN32;
#endif
#elif __APPLE__
        return OS_APPLE;
#elif __linux__
        return OS_LINUX;
#elif __unix__
        return OS_UNIX;
#else
        return OS_UNKNOWN;
#endif
    }

    OSVersion get_version() {
        OSVersion version = {0, 0};

#ifdef _WIN32
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
#elif __APPLE__
        char str[256];
        size_t size = sizeof(str);
        if (sysctlbyname("kern.osrelease", &str, &size, NULL, 0) == 0) {
            sscanf(str, "%d.%d", &version.major, &version.minor);
        }
#elif __linux__ || __unix__
        struct utsname buffer;
        if (uname(&buffer) == 0) {
            sscanf(buffer.release, "%d.%d", &version.major, &version.minor);
        }
#endif
        return version;
    }

    u64 get_uptime_ms() {
#ifdef _WIN32
        return static_cast<u64>(GetTickCount64());
#elif __linux__ || __unix__
        struct timespec ts;
        if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
            return 0;
        }
        return static_cast<u64>(ts.tv_sec) * 1000 + ts.tv_nsec / 1000000;
#elif __APPLE__
        struct timeval boottime;
        size_t size = sizeof(boottime);
        int mib[2] = {CTL_KERN, KERN_BOOTTIME};

        if (sysctl(mib, 2, &boottime, &size, NULL, 0) != 0) {
            return 0;
        }

        struct timeval now;
        gettimeofday(&now, NULL);

        u64 uptime_sec = now.tv_sec - boottime.tv_sec;
        i64 uptime_usec = now.tv_usec - boottime.tv_usec;

        if (uptime_usec < 0) {
            uptime_sec -= 1;
            uptime_usec += 1000000;
        }

        u64 uptime_ms = uptime_sec * 1000 + uptime_usec / 1000;
        return uptime_ms;
#endif
    }

    MemInfo get_meminfo() {
        MemInfo memInfo = {0};
#ifdef __linux__
        struct sysinfo info;
        if (sysinfo(&info) != 0) {
            std::cerr << "Can't get memory info" << std::endl;
            return memInfo;
        }
        memInfo.total_bytes = info.totalram * info.mem_unit;
        memInfo.free_bytes = info.freeram * info.mem_unit;
        memInfo.available_bytes = info.freeram * info.mem_unit + info.bufferram * info.mem_unit;
#elif __APPLE__
        int mibTotal[2] = {CTL_HW, HW_MEMSIZE};
        u64 totalMemory = 0;
        size_t size = sizeof(totalMemory);
        if (sysctl(mibTotal, 2, &totalMemory, &size, NULL, 0) != 0) {
            std::cerr << "Can't get memory info" << std::endl;
            return memInfo;
        }
        memInfo.total_bytes = totalMemory;

        mach_port_t machPort = mach_host_self();
        mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
        vm_statistics64_data_t vmStats;
        if (host_statistics64(machPort, HOST_VM_INFO, reinterpret_cast<host_info64_t>(&vmStats), &count) != KERN_SUCCESS) {
            std::cerr << "Can't get memory info" << std::endl;
            return memInfo;
        }

        u64 freeMemory = static_cast<u64>(vmStats.free_count) * sysconf(_SC_PAGESIZE);
        u64 activeMemory = static_cast<u64>(vmStats.active_count) * sysconf(_SC_PAGESIZE);
        u64 inactiveMemory = static_cast<u64>(vmStats.inactive_count) * sysconf(_SC_PAGESIZE);
        u64 speculativeMemory = static_cast<u64>(vmStats.speculative_count) * sysconf(_SC_PAGESIZE);

        memInfo.free_bytes = freeMemory;
        memInfo.available_bytes = activeMemory + inactiveMemory + speculativeMemory;
#elif _WIN32
        MEMORYSTATUSEX status;
        status.dwLength = sizeof(status);
        if (!GlobalMemoryStatusEx(&status)) {
            std::cerr << "Can't get memory info" << std::endl;
            return memInfo;
        }

        memInfo.total_bytes = status.ullTotalPhys;
        memInfo.free_bytes = status.ullAvailPhys;
#endif
        return memInfo;
    }

    std::vector<DriveInfo> get_drives() {
        std::vector<DriveInfo> drives;

#ifdef _WIN32
        // Получение маски логических дисков
        DWORD driveMask = GetLogicalDrives();
        if (driveMask == 0) {
            std::cerr << "GetLogicalDrives() failed with error " << GetLastError() << std::endl;
            return drives;
        }

        // Итерация по буквам дисков от A до Z
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

                // Получение свободного места на диске
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

#elif defined(__linux__)
        FILE *mounts = setmntent("/proc/mounts", "r");
        if (mounts == nullptr) {
            std::cerr << "Failed to open /proc/mounts." << std::endl;
            return drives;
        }

        struct mntent *ent;
        while ((ent = getmntent(mounts)) != nullptr) {
            std::string fsname = ent->mnt_fsname;
            std::string dir = ent->mnt_dir;
            std::string type = ent->mnt_type;

            DriveType driveType = DRIVE_TYPE_UNKNOWN;

            if (type == "nfs" || type == "cifs" || type == "smbfs" || type == "afs" || type == "sshfs") {
                driveType = DRIVE_TYPE_NET;
            }
            else if (dir.find("/media/") == 0 || dir.find("/mnt/") == 0) {
                driveType = DRIVE_TYPE_REMOVABLE;
            }
            else if (type == "ext4" || type == "xfs" || type == "btrfs" || type == "apfs" || type == "hfs" || type == "hfs+") {
                driveType = DRIVE_TYPE_LOCAL;
            }
            else {
                driveType = DRIVE_TYPE_FS;
            }

            struct statvfs stat;
            uint64_t freeBytes = 0;
            if (statvfs(dir.c_str(), &stat) == 0) {
                freeBytes = static_cast<uint64_t>(stat.f_bavail) * stat.f_frsize;
            } else {
                std::cerr << "statvfs() failed for " << dir << " with error " << strerror(errno) << std::endl;
            }

            drives.emplace_back(DriveInfo{driveType, dir, freeBytes});
        }

        endmntent(mounts);

#elif defined(__APPLE__)
        struct statfs *mounts;
        int count = getmntinfo(&mounts, MNT_NOWAIT);
        if (count == 0) {
            std::cerr << "getmntinfo failed." << std::endl;
            return drives;
        }

        for (int i = 0; i < count; ++i) {
            std::string fsname = mounts[i].f_mntfromname;
            std::string dir = mounts[i].f_mntonname;
            std::string type = mounts[i].f_fstypename;

            DriveType driveType = DRIVE_TYPE_UNKNOWN;

            if (type == "nfs" || type == "smbfs" || type == "afpfs") {
                driveType = DRIVE_TYPE_NET;
            }
            else if (dir.find("/Volumes/") == 0 && fsname.find("/dev/") == 0) {
                driveType = DRIVE_TYPE_REMOVABLE;
            }
            else if (type == "apfs" || type == "hfs" || type == "hfs+") {
                driveType = DRIVE_TYPE_LOCAL;
            }
            else {
                driveType = DRIVE_TYPE_FS;
            }

            struct statvfs stat;
            uint64_t freeBytes = 0;
            if (statvfs(dir.c_str(), &stat) == 0) {
                freeBytes = static_cast<uint64_t>(stat.f_bavail) * stat.f_frsize;
            } else {
                std::cerr << "statvfs() failed for " << dir << " with error " << strerror(errno) << std::endl;
            }

            drives.emplace_back(DriveInfo{driveType, dir, freeBytes});
        }
#endif

        return drives;
    }

    AccessRightsInfo get_access_info(const std::wstring &path) {
        AccessRightsInfo rightsInfo = {};

#ifdef _WIN32
        PSECURITY_DESCRIPTOR securityDescriptor = nullptr;
        PACL dacl = nullptr;

        if (GetNamedSecurityInfoW(
                path.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION,
                nullptr, nullptr, &dacl, nullptr, &securityDescriptor) != ERROR_SUCCESS) {
            std::wcerr << L"Failed to get security descriptor. Error: " << GetLastError() << std::endl;
            return rightsInfo;
        }

        if (!dacl) {
            std::wcerr << L"No DACL present for the object: " << path << std::endl;
            LocalFree(securityDescriptor);
            return rightsInfo;
        }

        for (DWORD i = 0; i < dacl->AceCount; i++) {
            PACE_HEADER aceHeader = nullptr;
            if (!GetAce(dacl, i, reinterpret_cast<void **>(&aceHeader))) {
                std::wcerr << L"Failed to get ACE at index " << i << L". Error: " << GetLastError() << std::endl;
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
#endif


        return rightsInfo;
    }

    OwnerInfo get_owner_info(const std::wstring &path) {
        OwnerInfo ownerInfo = {};
#ifdef _WIN32
        PSECURITY_DESCRIPTOR securityDescriptor = nullptr;
        PSID ownerSid = nullptr;

        if (GetNamedSecurityInfoW(
                path.c_str(), SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION,
                &ownerSid, nullptr, nullptr, nullptr, &securityDescriptor) != ERROR_SUCCESS) {
            return ownerInfo;
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
#endif

        return ownerInfo;
    }
}
