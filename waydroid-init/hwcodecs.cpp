/*
 * Copyright (C) 2026 The WayDroid-ATV Project
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <string>

#include "log.h"
#include "hwcodecs.h"

HWCodecs::HWCodecs([[maybe_unused]] const std::string &renderNode) {
#ifdef USE_VAAPI
    getHWCodecListVAAPI(renderNode);
#else
    getHWCodecListV4L2();
#endif
}

std::string HWCodecs::getAvailHWCodecs() {
    std::string result, codec;

    Log::info("Supported HW codec(s) by GPU:");
    Log::info("");

    for (const auto &fourCC : availHWCodecs) {
        Log::info("  {}", codec = convertFourCCToString(fourCC));
        result += codec;
    }

    Log::info("");
    return result;
}


std::string HWCodecs::getAvailHWCodecProfiles() {
    std::string result;

    Log::info("Supported HW codec profile(s) by GPU:");
    Log::info("");

    auto appendProfile = [&](const std::string &profile) {
        Log::info("  {}", profile);
        if (!result.empty()) result += ",";
        result += profile;
    };

    if (hevcMain10) appendProfile("HEVCMain10");
    if (vp9Profile2) appendProfile("VP9Profile2");
    if (av1Profile0) appendProfile("AV1Profile0");

    Log::info("");
    return result;
}
