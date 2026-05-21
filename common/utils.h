#pragma once
#include <string>
#include <cstdint>
#include <sys/types.h>

namespace Utils {
    bool createBindMount(const std::string &src, const std::string &dst, uint64_t flags);
    bool setFilePermission(const std::string &file, uid_t owner, gid_t group, mode_t mode);
}
