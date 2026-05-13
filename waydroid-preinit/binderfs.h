/*
 * Copyright (C) 2026 The WayDroid-ATV Project
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <string>
#include <cstdint>
#include <linux/limits.h>
#include <sys/ioctl.h>

#define BINDER_CTL_ADD _IOWR('b', 1, binderNode)

namespace BinderFS {
    struct binderNode {
        char     name[NAME_MAX + 1];
        uint32_t major;
        uint32_t minor;
    };

    void createDevice();
}
