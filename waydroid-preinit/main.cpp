/*
 * Copyright (C) 2026 The WayDroid-ATV Project
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <array>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <system_error>
#include <filesystem>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/sysmacros.h>

#include "log.h"
#include "binderfs.h"
#include "dev_node.h"
#include "utils.h"
#include "waydroid_prop.h"

#define LOG_TAG "waydroid-preinit"

using namespace std;

static inline bool moveDockerEtc() {
    error_code error;

    if (mkdir("/dev/.docker", 0755) == -1) {
        Log::err("Failed to create /dev/.docker: {}", strerror(errno));
        return false;
    }

    for (const auto &entry : filesystem::directory_iterator("/etc")) {
        string dest = "/dev/.docker/" + entry.path().filename().string();

        // Do not move directories
        if (entry.is_directory()) continue;

        // Copy file
        filesystem::copy(entry.path(), dest, error);
        if (error.value()) {
            Log::err("Failed to copy {}: {}", entry.path().string(), error.message());
            return false;
        }

        // Unmount the file, it is okay for umount() to fail
        umount2(entry.path().c_str(), MNT_DETACH);

        // Remove source file
        if (remove(entry.path().c_str()) == -1) {
            Log::err("Failed to remove {}: {}", entry.path().string(), strerror(errno));
            return false;
        }
    }

    // Remove /etc directory
    if (filesystem::remove_all("/etc", error) == static_cast<uintmax_t>(-1)) {
        Log::err("Failed to remove /etc: {}", error.message());
        return false;
    }

    // Restore /etc -> /system/etc symlink
    if (symlink("/system/etc", "/etc") == -1) {
        Log::err("Failed to creare symlink for /etc: {}", strerror(errno));
        return false;
    }

    return true;
}

static inline bool remountProc() {
    int sysNetDirFd;

    // Backup RW copy of /proc/sys/net
    if ((sysNetDirFd = syscall(SYS_open_tree, AT_FDCWD, "/proc/sys/net", OPEN_TREE_CLONE)) == -1) {
        Log::err("open_tree() failed for /proc/sys/net: {}", strerror(errno));
        return false;
    }

    // Remount /proc/sysrq-trigger and /proc/sys as read-only
    if (!(
        Utils::createBindMount("/proc/sys", "/proc/sys", MS_RDONLY) &&
        Utils::createBindMount("/proc/sysrq-trigger", "/proc/sysrq-trigger", MS_RDONLY)
    )) goto failed;

    // Mount detached RW copy of /proc/sys/net
    if (syscall(SYS_move_mount, sysNetDirFd, "", AT_FDCWD, "/proc/sys/net", MOVE_MOUNT_F_EMPTY_PATH) == -1) {
        Log::err("Failed to mount /proc/sys/net: {}", strerror(errno));
        goto failed;
    }

    close(sysNetDirFd);
    return true;

failed:
    close(sysNetDirFd);
    return false;
}

int main(int argc, char **argv) {
    error_code     error;
    vector<string> argsArray(argv, argv + argc);
    WaydroidProp   properties(argsArray);

    umask(0);

    if (getuid() != 0) {
        Log::err("Must be run with root");
        return 1;
    }

    // Create /dev/kmsg if not exist
    if (!filesystem::exists("/dev/kmsg")) {
        if (mknod("/dev/kmsg", S_IFCHR | 0600, makedev(1, 11)) == -1) {
            Log::err("Failed to create /dev/kmsg: {}", strerror(errno));
            return errno;
        }
    }

    Log::init(LOG_TAG);

    // Check if we are running inside docker
    if (filesystem::exists("/.dockerenv")) {
        Log::info("Running in Docker");
        properties.setProperty("ro.waydroid.running_in_docker", "1");

        // Recreate /dev to ensure correct file permission
        if (umount2("/dev", MNT_DETACH) == -1) {
            Log::err("Failed to unmount /dev: {}", strerror(errno));
            return errno;
        }

        if (mount("tmpfs", "/dev", "tmpfs", MS_NOSUID, "mode=0755") == -1) {
            Log::err("Failed to mount /dev: {}", strerror(errno));
            return errno;
        }

        if (mkdir("/dev/pts", 0755) == -1) {
            Log::err("Failed to create /dev/pts: {}", strerror(errno));
            return errno;
        }

        if (mount("devpts", "/dev/pts", "devpts", 0, NULL) == -1) {
            Log::err("Failed to mount /dev/pts: {}", strerror(errno));
            return errno;
        }

        // Move /etc and restore /system/etc -> /etc symlink
        if (!moveDockerEtc()) return errno;
    } else {
        // Remove /dev bind-mounts for security
        DevNode::fixBindMountedNodes();

        // Undo LXC /proc read-only layers (we will set up again later)
        umount2("/proc/sys", MNT_DETACH);
        umount2("/proc/sysrq-trigger", MNT_DETACH);
    }

    // Fix directory/node permissions
    DevNode::fixPermission();

    // Create binder devices if not exist
    if (!filesystem::is_character_file("/dev/binder", error)) BinderFS::createDevice();

    // Clean /dev/input/*, otherwise inputflinger will try to access input devices from host
    if (filesystem::is_directory("/dev/input", error)) {
        if (filesystem::remove_all("/dev/input", error) == static_cast<uintmax_t>(-1)) {
            Log::err("Failed to clean /dev/input: {}", error.message());
            return error.value();
        }
    }

    // Remount /sys as read-only to prevent unexpected issues
    if (mount("sysfs", "/sys", "sysfs", MS_REMOUNT | MS_RDONLY, NULL) == -1) {
        Log::err("Failed to remount sysfs: {}", strerror(errno));
        return errno;
    }

    // Remount /proc/sysrq-trigger and /proc/sys (except /proc/sys/net) as read-only for security
    if (!remountProc()) return errno;

    // Write props back to waydroid.prop
    properties.saveProperties();
}
