/*
 * Copyright (C) 2026 The WayDroid-ATV Project
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <string>
#include <filesystem>
#include <format>
#include <fstream>
#include <sys/sysinfo.h>

#include "cpu_utils.h"

CpuUtils::CpuUtils() {
    int totalCores = get_nprocs_conf();
    this->allCores = std::format("0-{}", totalCores - 1);

    if (!std::filesystem::exists("/sys/devices/cpu_core")) {
        // Use all cores for efficient mode on AMD/non-hybrid Intel CPUs
        // TODO: Check for c variant cores on recent Ryzen CPUs
        this->efficientCores = allCores;
        return;
    }

    // Read available E cores from sysfs
    std::string   buf;
    std::ifstream eCoreListFile("/sys/devices/cpu_atom/cpus"),       // E cores
                  lpeCoreListFile("/sys/devices/cpu_lowpower/cpus"); // Low-power E cores

    if (eCoreListFile.is_open()) eCoreListFile >> this->efficientCores;

    // Append low-power E cores to efficient core list if there are any
    if (lpeCoreListFile.is_open()) {
        lpeCoreListFile >> buf;
        this->efficientCores += "," + buf;
    }
}
