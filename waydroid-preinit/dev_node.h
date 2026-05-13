/*
 * Copyright (C) 2026 The WayDroid-ATV Project
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <array>
#include <string>
#include <sys/stat.h>

namespace DevNode {
    struct entryInfo {
        std::string path;
        uid_t       owner;
        gid_t       group;
        mode_t      mode;
    };

    const std::array<std::string, 3> binderNodes = {
        "/dev/binder",
        "/dev/hwbinder",
        "/dev/vndbinder",
    };

    void fixPermission();
    void fixBindMountedNodes();
}
