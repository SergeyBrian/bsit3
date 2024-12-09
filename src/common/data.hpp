#ifndef DATA_H
#define DATA_H

#include "alias.hpp"
#include <string>
#include <vector>
#include <array>

enum OSType {
    OS_WIN32,
    OS_WIN64,
    OS_APPLE,
    OS_LINUX,
    OS_UNIX,
    OS_UNKNOWN
};

inline const char *OSTypeName[] = {
        "Windows 32-bit",
        "Windows 64-bit",
        "Mac OSX",
        "Linux",
        "Unix",
        "Unknown OS"
};

struct OSVersion {
    u16 major;
    u16 minor;
};

struct MemInfo {
    u64 total_bytes;
    u64 free_bytes;
    u64 available_bytes;
};

enum DriveType {
    DRIVE_TYPE_LOCAL,
    DRIVE_TYPE_NET,
    DRIVE_TYPE_REMOVABLE,
    DRIVE_TYPE_FS,
    DRIVE_TYPE_UNKNOWN
};

inline const char *DriveTypeName[] = {
        "local",
        "network",
        "removable",
        "file system",
        "unknown"
};

struct DriveInfo {
    DriveType type;
    std::string name;
    u64 free_bytes;
};

enum AceType : uint8_t {
    ACE_TYPE_ALLOWED = 0,
    ACE_TYPE_DENIED = 1,
    ACE_TYPE_OTHER = 2,
};

enum Scope : uint8_t {
    SCOPE_DIRECT = 0,
    SCOPE_OBJECT = 1,
    SCOPE_CONTAINER = 2,
};

struct AccessControlEntry {
    std::array<uint8_t, 32> sid;
    AceType aceType;
    Scope scope;
    uint32_t accessMask;
};

struct AccessRightsInfo {
    std::vector<AccessControlEntry> entries;
};

struct OwnerInfo {
    std::string ownerName;
    std::string ownerDomain;
};

#endif
