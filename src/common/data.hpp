#ifndef DATA_H
#define DATA_H

#include "alias.hpp"
#include <string>
#include <vector>
#include <array>

enum OSType : u8 { OS_WIN32, OS_WIN64, OS_WINARM, OS_WINARM64, OS_UNKNOWN };

inline const char *OSTypeName[] = {
    "Windows x86", "Windows x64", "Windows ARM", "Windows ARM64", "Windows",
};

struct OSVersion {
    u16 major;
    u16 minor;
};

struct OSInfo {
    OSType type;
    OSVersion version;
};

struct MemInfo {
    u64 total_bytes;
    u64 free_bytes;
};

enum DriveType : u8 {
    DRIVE_TYPE_LOCAL,
    DRIVE_TYPE_NET,
    DRIVE_TYPE_REMOVABLE,
    DRIVE_TYPE_FS,
    DRIVE_TYPE_UNKNOWN
};

inline const char *DriveTypeName[] = {"local", "network", "removable",
                                      "file system", "unknown"};

struct DriveInfo {
    DriveType type;
    std::string name;
    u64 free_bytes;
};

enum AceType : u8 {
    ACE_TYPE_ALLOWED = 0,
    ACE_TYPE_DENIED = 1,
    ACE_TYPE_OTHER = 2,
};

enum Scope : u8 {
    SCOPE_DIRECT = 0,
    SCOPE_OBJECT = 1,
    SCOPE_CONTAINER = 2,
};

struct AccessControlEntry {
    std::array<u8, 32> sid;
    AceType aceType;
    Scope scope;
    u32 accessMask;
};

struct AccessRightsInfo {
    std::vector<AccessControlEntry> entries;
};

struct OwnerInfo {
    std::string ownerName;
    std::string ownerDomain;
    std::array<u8, 32> sid;
};

#endif
