/*
 * Copyright (C) 2026 The WayDroid-ATV Project
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <linux/videodev2.h>

#ifndef V4L2_PIX_FMT_AV1
#define V4L2_PIX_FMT_AV1 v4l2_fourcc('A', 'V', '1', '0')
#endif

#ifndef V4L2_PIX_FMT_AV1_FRAME
#define V4L2_PIX_FMT_AV1_FRAME v4l2_fourcc('A', 'V', '1', 'F')
#endif

class HWCodecs {
    private:
        std::vector<uint32_t> availHWCodecs;
        bool hevcMain10 = false;
        bool vp9Profile2 = false;
        bool av1Profile0 = false;

        inline std::string convertFourCCToString(uint32_t fourCC) {
            return std::string(reinterpret_cast<const char *>(&fourCC), 4);
        }

#ifdef USE_VAAPI
        void getHWCodecListVAAPI(const std::string &renderNode);
#else
        void getHWCodecListV4L2();
#endif

    public:
        HWCodecs([[maybe_unused]] const std::string &renderNode);
        std::string getAvailHWCodecs();
        std::string getAvailHWCodecProfiles();
        bool supportsHevcMain10() const { return hevcMain10; }
        bool supportsVp9Profile2() const { return vp9Profile2; }
        bool supportsAv1Profile0() const { return av1Profile0; }
};
