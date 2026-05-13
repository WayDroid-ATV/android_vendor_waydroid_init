#include <string>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "log.h"
#include "utils.h"

namespace Utils {
    bool setFilePermission(const std::string &file, uid_t owner, gid_t group, mode_t mode) {
        if (chown(file.c_str(), owner, group) == -1) {
            Log::warn("chown() failed for {}: {}", file, strerror(errno));
            return false;
        }

        if (chmod(file.c_str(), mode) == -1) {
            Log::warn("chmod() failed for {}: {}", file, strerror(errno));
            return false;
        }

        return true;
    }
}
