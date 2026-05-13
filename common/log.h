/*
 * Copyright (C) 2026 The WayDroid-ATV Project
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <format>
#include <utility>

namespace Log {
    enum LogLevel {
        LOG_ERR  = 3,
        LOG_WARN = 4,
        LOG_INFO = 6,
    };

    void init(const std::string &tag);
    void printMessage(const enum LogLevel &level, const std::string &message, const std::format_args &args);

    template<typename... Args>
    inline void info(const std::string &message, Args&&... args) {
        printMessage(LOG_INFO, message, std::make_format_args(args...));
    }

    template<typename... Args>
    inline void warn(const std::string &message, Args&&... args) {
        printMessage(LOG_WARN, message, std::make_format_args(args...));
    }

    template<typename... Args>
    inline void err(const std::string &message, Args&&... args) {
        printMessage(LOG_ERR, message, std::make_format_args(args...));
    }
}
