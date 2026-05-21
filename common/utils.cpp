#include <string>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "log.h"
#include "utils.h"

namespace Utils {
    bool createBindMount(const std::string &src, const std::string &dst, uint64_t flags) {
        if (mount(src.c_str(), dst.c_str(), "", MS_BIND | flags, NULL) == -1) {
            Log::err("mount {} -> {}: mount() failed: {}", src, dst, strerror(errno));
            return false;
        }

        // Remount as read-only if needed
        if ((flags & MS_RDONLY) && mount(src.c_str(), dst.c_str(), "", MS_REMOUNT | MS_BIND | MS_RDONLY, NULL) != 0) {
            Log::err("remount() failed for {}: {}", dst, strerror(errno));
            return false;
        }

        return true;
    }

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
