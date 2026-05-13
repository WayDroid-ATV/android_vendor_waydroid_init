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
