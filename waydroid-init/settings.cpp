/*
 * Copyright (C) 2026 The WayDroid-ATV Project
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <iostream>
#include <fstream>
#include <map>
#include <cerrno>
#include <cstring>
#include <android-base/properties.h>

#include "log.h"
#include "settings.h"

WaydroidSettings::WaydroidSettings(const std::string &settingsPath, bool overrideProps) {
    std::ifstream settingsFile;
    std::string   line;
    size_t        delim;

    this->overrideProps = overrideProps;
    Log::info("Using settings file: {}", settingsPath);

    settingsFile.open(settingsPath);
    if (!settingsFile.is_open()) {
        Log::err("Failed to read settings file: {}", strerror(errno));
        return;
    }

    // Parse settings file
    while (std::getline(settingsFile, line)) {
        if ((delim = line.find('=')) == std::string::npos) continue;

        // Only load recognized props
        std::string key = line.substr(0, delim);
        if (!key.starts_with("#") && defaultSettings.contains(key)) overrideSettings[key] = line.substr(delim + 1);
    }
}

std::string WaydroidSettings::getSetting(const std::string &prop) const {
    auto overrideIt = overrideSettings.find(prop);
    auto defaultIt  = defaultSettings.find(prop);

    if (overrideIt != overrideSettings.end()) {
        return overrideIt->second;
    } else if (defaultIt != defaultSettings.end()) {
        return android::base::GetProperty(prop, defaultIt->second);
    } else {
        return android::base::GetProperty(prop, "");
    }
}

bool WaydroidSettings::getBoolSetting(const std::string &prop) const {
    std::string result = getSetting(prop);
    return (result == "1" || result == "true");
}

void WaydroidSettings::updateSetting(const std::string &prop, const std::string &value) {
    // Do not override props set in /data/misc/waydroid-settings
    if (!overrideSettings.contains(prop)) {
        if (overrideProps) {
            overrideSettings[prop] = value;
        } else {
            overrideSettings[prop] = android::base::GetProperty(prop, value);
        }
    }
}

void WaydroidSettings::loadSettings() {
    std::map<std::string, std::string> mergedSettings(overrideSettings);

    for (const auto &[prop, defaultValue] : defaultSettings) {
        if (!mergedSettings.contains(prop)) {
            mergedSettings[prop] = android::base::GetProperty(prop, defaultValue);
        }
    }

    Log::info("Loaded properties:");
    Log::info("");

    for (const auto &[key, value] : mergedSettings) {
        android::base::SetProperty(key, value);
        Log::info("  {}={}", key, value);
    }

    Log::info("");
}
