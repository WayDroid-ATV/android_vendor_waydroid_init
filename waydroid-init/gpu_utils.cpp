/*
 * Copyright (C) 2026 The WayDroid-ATV Project
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#define _XOPEN_SOURCE 500
#include <iostream>
#include <fstream>
#include <format>
#include <string>
#include <system_error>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <unistd.h>
#include <sys/sysmacros.h>

#include "log.h"
#include "gpu_utils.h"

void GpuUtils::getKernelDriver() {
    std::error_code       error;
    std::string           symlinkPath = std::format("/sys/dev/char/{}:{}/device/driver", major(this->renderNodeInfo.st_rdev), minor(this->renderNodeInfo.st_rdev));
    std::filesystem::path driverPath  = std::filesystem::read_symlink(symlinkPath, error);

    if (driverPath.empty()) {
        Log::err("Failed to read GPU driver name: {}", error.message());
        return;
    }

    this->gpuKernelDriverName = driverPath.filename();
    this->gpuRenderNodeMinor  = minor(this->renderNodeInfo.st_rdev);
}

float GpuUtils::getIntelGpuGeneration() {
    std::string   line;
    std::ifstream capabilities;
    size_t        delim;

    capabilities.open(std::format("/sys/kernel/debug/dri/{}/i915_capabilities", minor(this->renderNodeInfo.st_rdev)), std::ios::in);

    if (!capabilities.is_open()) {
        Log::err("Failed to read GPU capabilities: {}", strerror(errno));
        return -errno;
    }

    while (std::getline(capabilities, line)) {
        if ((delim = line.find(':')) == std::string::npos) {
            continue;
        } else if (line.substr(0, delim) == "graphics version") {
            return std::stof(line.substr(delim + 2));
        }
    }

    return -1;
}

GpuUtils::GpuUtils(const std::string &renderNode, bool swRendering) {
    float gpuGeneration;

    if (swRendering) goto swFallback;

    this->renderNode = renderNode;

    if (stat(renderNode.c_str(), &this->renderNodeInfo) == -1) {
        Log::err("Failed to access GPU render node: {}", strerror(errno));
        goto swFallback;
    }

    if (!S_ISCHR(this->renderNodeInfo.st_mode)) {
        Log::err("GPU render node is not a character device");
        goto swFallback;
    }

    getKernelDriver();

    gpuGeneration = gpuKernelDriverName == "i915" ? getIntelGpuGeneration() : 0;

    for (const auto &e : driverInfoList) {
        if (e.driverName == gpuKernelDriverName) {
            if (e.minGeneration && e.minGeneration > gpuGeneration) continue;

            Log::info("GPU kernel driver: {}", e.driverName);
            this->driverInfo = e;
            return;
        }
    }

    goto swFallback;

swFallback:
    Log::err("GPU driver unsupported or unrecognized, fallback to software rendering");
    this->driverInfo = driverInfoList.back();
}
