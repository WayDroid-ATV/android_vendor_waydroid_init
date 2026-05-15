/*
 * Copyright (C) 2026 The WayDroid-ATV Project
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <string>
#include <filesystem>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include "log.h"
#include "dev_node.h"
#include "binderfs.h"

namespace BinderFS {
    static int controlFd = -1;

    static bool mountFS() {
        std::filesystem::create_directory("/dev/binderfs");

        if (mount("binder", "/dev/binderfs", "binder", 0, "stats=global") == -1) {
            Log::err("Failed to mount binderfs (is the binder kernel module loaded?): {}", strerror(errno));
            return false;
        }

        if (chmod("/dev/binderfs/binder-control", 0600) == -1) {
            Log::warn("Failed to set permission for binder controller: {}", strerror(errno));
        }

        return true;
    }

    static bool openController() {
        if ((controlFd = open("/dev/binderfs/binder-control", O_RDONLY | O_CLOEXEC)) == -1) {
            Log::err("Failed to open binder control device: {}", strerror(errno));
            return false;
        }

        return true;
    }

    void createDevice() {
        binderNode nodeInfo;

        if (!mountFS()) return;
        if (!openController()) return;

        for (const auto &symlinkPath : DevNode::binderNodes) {
            std::string nodeName   = symlinkPath.substr(5),
                        devicePath = "/dev/binderfs/" + nodeName;

            if (std::filesystem::exists(symlinkPath)) continue;

            strlcpy(nodeInfo.name, nodeName.c_str(), NAME_MAX);

            if (ioctl(controlFd, BINDER_CTL_ADD, &nodeInfo) == -1) {
                Log::warn("Failed to allocate new binder device: {}", strerror(errno));
                continue;
            }

            // Setup permission
            if (chmod(devicePath.c_str(), 0666) == -1) {
                Log::warn("Failed to set permission for binder device {}: {}", devicePath, strerror(errno));
            }

            // Create symlink in /dev for allocated device
            if (symlink(devicePath.c_str(), symlinkPath.c_str()) == -1) {
                Log::warn("Failed to create symlink for binder device {}: {}", devicePath, strerror(errno));
            }
        }
    }
}
