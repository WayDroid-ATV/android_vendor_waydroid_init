/*
 * Copyright (C) 2026 The WayDroid-ATV Project
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef USE_VAAPI
#include <algorithm>
#include <format>
#include <string>
#include <filesystem>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "log.h"
#include "hwcodecs.h"

void HWCodecs::getHWCodecListV4L2() {
    std::string videoDevice;

    struct v4l2_capability cap;
    struct v4l2_fmtdesc    fmt;

    int fd, ret;

    for (const auto &entry : std::filesystem::directory_iterator("/dev")) {
        videoDevice = entry.path().filename().string();

        if (!videoDevice.starts_with("video")) continue;

        Log::info("Open V4L2 device {}", videoDevice);

        if ((fd = open(entry.path().c_str(), O_RDONLY | O_CLOEXEC)) == -1) {
            Log::warn("Failed to open V4L2 device {}: {}", videoDevice, strerror(errno));
            continue;
        }

        if (ioctl(fd, VIDIOC_QUERYCAP, &cap) != 0) {
            Log::warn("ioctl(VIDIOC_QUERYCAP) failed for {}: {}", videoDevice, strerror(errno));
            close(fd);
            continue;
        }

        if (!(cap.capabilities & (V4L2_CAP_VIDEO_M2M | V4L2_CAP_VIDEO_M2M_MPLANE))) {
            close(fd);
            continue;
        }

        Log::info("V4L2 M2M device found");

        // Query for supported codecs
        fmt.index = 0;
        fmt.type  = (cap.capabilities & V4L2_CAP_VIDEO_M2M_MPLANE) ? V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE : V4L2_BUF_TYPE_VIDEO_OUTPUT;

        do {
            if ((ret = ioctl(fd, VIDIOC_ENUM_FMT, &fmt)) == -1 && errno == EINVAL) {
                break;
            } else if (ret == -1) {
                Log::warn("ioctl(VIDIOC_ENUM_FMT) failed for {}: {}", videoDevice, strerror(errno));
                continue;
            }

            switch (fmt.pixelformat) {
                case V4L2_PIX_FMT_MPEG2:
                case V4L2_PIX_FMT_MPEG2_SLICE:
                case V4L2_PIX_FMT_MPEG4:
                case V4L2_PIX_FMT_H263:
                case V4L2_PIX_FMT_H264:
                case V4L2_PIX_FMT_H264_SLICE:
                case V4L2_PIX_FMT_HEVC:
                case V4L2_PIX_FMT_HEVC_SLICE:
                case V4L2_PIX_FMT_VP8:
                case V4L2_PIX_FMT_VP8_FRAME:
                case V4L2_PIX_FMT_VP9:
                case V4L2_PIX_FMT_VP9_FRAME:
                case V4L2_PIX_FMT_AV1_FRAME:
                    break;
                default:
                    continue;
            }


            if (std::find(this->availHWCodecs.begin(), this->availHWCodecs.end(), fmt.pixelformat) == this->availHWCodecs.end()) {
                this->availHWCodecs.emplace_back(fmt.pixelformat);
            }
        } while (++fmt.index);

        close(fd);
    }
}
#endif
