#pragma once
#include <string>
#include <sys/types.h>

namespace Utils {
    bool setFilePermission(const std::string &file, uid_t owner, gid_t group, mode_t mode);
}
