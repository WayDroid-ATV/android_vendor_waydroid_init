/*
 * Copyright (C) 2026 The WayDroid-ATV Project
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <string>
#include <map>
#include <android/api-level.h>

#if __ANDROID_API__ <= 34
#define NATIVE_BRIDGE_LIB "libhoudini.so"
#else
#define NATIVE_BRIDGE_LIB "libndk_translation.so"
#endif

class WaydroidSettings {
    private:
        const std::map<std::string, std::string> defaultSettings = {
            { "debug.ffmpeg-codec2.hwaccel.drm",   "0"                   },
            { "debug.ffmpeg-codec2.pixel_format",  "YUV_420"             },
            { "gralloc.gbm.device",                "/dev/dri/renderD128" },
            { "media.sf.hwaccel",                  "0"                   },
            { "ro.dalvik.vm.native.bridge",        NATIVE_BRIDGE_LIB     },
            { "ro.hardware.egl",                   "angle"               },
            { "ro.hardware.gralloc",               "default"             },
            { "ro.hardware.hwcomposer",            "waydroid"            },
            { "ro.hardware.vulkan",                "pastel"              },
            { "ro.waydroid.codec2-impl",           "c2.ffmpeg"           },
            { "ro.waydroid.forward_notifications", "0"                   },
            { "ro.waydroid.google_tv_mode",        "0"                   },
            { "ro.waydroid.software_rendering",    "0"                   },
        };

        std::map<std::string, std::string> overrideSettings;
        bool overrideProps;
    public:
        WaydroidSettings(const std::string &settingsPath, bool overrideProps);
        std::string getSetting(const std::string &prop) const;
        bool getBoolSetting(const std::string &prop) const;
        void updateSetting(const std::string &prop, const std::string &value);
        void loadSettings();
};
