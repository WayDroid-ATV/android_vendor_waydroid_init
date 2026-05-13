/*
 * Copyright (C) 2026 The WayDroid-ATV Project
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <iostream>
#include <string>
#include <format>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>

#include "log.h"

namespace Log {
    static std::string tag    = "waydroid-init";
    static int         kmsgFd = -1;

    void init(const std::string &tag) {
        Log::tag = tag;

        if ((kmsgFd = open("/dev/kmsg", O_WRONLY | O_CLOEXEC)) == -1) {
			std::cerr << tag << ": Failed to open kmsg: " << strerror(errno) << std::endl;
		}
    }

    void printMessage(const enum LogLevel &level, const std::string &message, const std::format_args &args) {
        std::string formattedStr = std::vformat(tag + ": " + message, args),
                    kmsgStr      = std::format("<{}>{}\n", static_cast<int>(level), formattedStr);

        if (kmsgFd != -1) write(kmsgFd, kmsgStr.c_str(), kmsgStr.length());

        if (level == LOG_INFO) {
            std::cout << formattedStr << std::endl;
        } else {
            std::cerr << formattedStr << std::endl;
        }
    }
}
