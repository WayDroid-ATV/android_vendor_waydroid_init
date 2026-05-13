/*
 * Copyright (C) 2026 The WayDroid-ATV Project
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifdef USE_VAAPI
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>

extern "C" {
#include <va/va.h>
#include <va/va_drm.h>
}

#include "log.h"
#include "hwcodecs.h"

static const std::unordered_map<VAProfile, uint32_t> VAProfileFourCC {
    { VAProfileMPEG2Simple,             V4L2_PIX_FMT_MPEG2 },
    { VAProfileMPEG2Main,               V4L2_PIX_FMT_MPEG2 },
    { VAProfileMPEG4Simple,             V4L2_PIX_FMT_MPEG4 },
    { VAProfileMPEG4AdvancedSimple,     V4L2_PIX_FMT_MPEG4 },
    { VAProfileMPEG4Main,               V4L2_PIX_FMT_MPEG4 },
    { VAProfileH263Baseline,            V4L2_PIX_FMT_H263  },
    { VAProfileH264ConstrainedBaseline, V4L2_PIX_FMT_H264  },
    { VAProfileH264Main,                V4L2_PIX_FMT_H264  },
    { VAProfileH264High,                V4L2_PIX_FMT_H264  },
    { VAProfileHEVCMain,                V4L2_PIX_FMT_HEVC  },
    { VAProfileHEVCMain10,              V4L2_PIX_FMT_HEVC  },
    { VAProfileHEVCMain12,              V4L2_PIX_FMT_HEVC  },
    { VAProfileVP8Version0_3,           V4L2_PIX_FMT_VP8   },
    { VAProfileVP9Profile0,             V4L2_PIX_FMT_VP9   },
    { VAProfileVP9Profile1,             V4L2_PIX_FMT_VP9   },
    { VAProfileVP9Profile2,             V4L2_PIX_FMT_VP9   },
    { VAProfileVP9Profile3,             V4L2_PIX_FMT_VP9   },
    { VAProfileAV1Profile0,             V4L2_PIX_FMT_AV1   },
    { VAProfileAV1Profile1,             V4L2_PIX_FMT_AV1   },
    { VAProfileAV1Profile2,             V4L2_PIX_FMT_AV1   },
};

void HWCodecs::getHWCodecListVAAPI(const std::string &renderNode) {
    VAStatus     status;
    VADisplay    dpy;
    VAProfile    *profiles;
    VAEntrypoint *entrypoints;

    int renderFd, vaVersion[2], numProfiles, numEntrypoints;

    if ((renderFd = open(renderNode.c_str(), O_RDWR | O_CLOEXEC)) == -1) {
        Log::err("Failed to open GPU render node: {}", strerror(errno));
        return;
    }

    if ((dpy = vaGetDisplayDRM(renderFd)) == nullptr) {
        Log::err("Failed to open VADisplay");
        goto cleanup;
    }

    if ((status = vaInitialize(dpy, &vaVersion[0], &vaVersion[1])) != VA_STATUS_SUCCESS) {
        Log::err("vaInitialize() failed: {}", status);
        goto cleanup;
    }

    Log::info("VA-API version {}.{} ({})", vaVersion[0], vaVersion[1], vaQueryVendorString(dpy));

    profiles    = (VAProfile *) alloca(vaMaxNumProfiles(dpy) * sizeof(VAProfile));
    entrypoints = (VAEntrypoint *) alloca(vaMaxNumEntrypoints(dpy) * sizeof(VAEntrypoint));

    if ((status = vaQueryConfigProfiles(dpy, profiles, &numProfiles)) != VA_STATUS_SUCCESS) {
        Log::err("vaQueryConfigProfiles() failed: {}", status);
        goto cleanup;
    }

    for (int i = 0; i < numProfiles; i++) {
        if ((status = vaQueryConfigEntrypoints(dpy, profiles[i], entrypoints, &numEntrypoints)) != VA_STATUS_SUCCESS) {
            Log::err("vaQueryConfigEntrypoints() failed: {}", status);
            goto cleanup;
        }

        for (int j = 0; j < numEntrypoints; j++) {
            if (entrypoints[j] != VAEntrypointVLD) continue;

            auto it = VAProfileFourCC.find(profiles[i]);

            if (it != VAProfileFourCC.end()) {
                uint32_t fourCC = it->second;

                if (std::find(this->availHWCodecs.begin(), this->availHWCodecs.end(), fourCC) == this->availHWCodecs.end()) {
                    this->availHWCodecs.emplace_back(fourCC);
                }
            }
        }
    }

    goto cleanup;

cleanup:
    if (dpy) vaTerminate(dpy);
    close(renderFd);
}
#endif
