/*
 * Copyright (C) 2026 The WayDroid-ATV Project
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <array>
#include <string>
#include <optional>
#include <cstdint>
#include <sys/stat.h>

typedef struct {
    std::string                driverName;
    uint8_t                    minGeneration;
    std::string                eglImpl;
    std::optional<std::string> vkDriverName;
    std::string                preferredGbm;
} GpuDriverInfo;

class GpuUtils {
    private:
        std::string renderNode;
        uint8_t     gpuRenderNodeMinor = 0;

        struct stat renderNodeInfo;

        const std::array<GpuDriverInfo, 19> driverInfoList = {{
            { "asahi",       0, "mesa",  "asahi",        "minigbm_gbm_mesa" },
            { "v3d",         0, "mesa",  "broadcom",     "minigbm_gbm_mesa" },
            { "vc4",         0, "mesa",  "broadcom",     "minigbm_gbm_mesa" },
            { "msm",         0, "mesa",  "freedreno",    "minigbm_gbm_mesa" },
            { "msm_dpu",     0, "mesa",  "freedreno",    "minigbm_gbm_mesa" },
            { "i915",        9, "mesa",  "intel",        "minigbm",         },
            { "i915",        0, "mesa",  "intel_hasvk",  "minigbm",         },
            { "xe",          0, "mesa",  "intel",        "minigbm",         },
            { "nouveau",     0, "mesa",  "nouveau",      "minigbm",         },
            { "lima",        0, "mesa",  std::nullopt,   "minigbm_gbm_mesa" },
            { "panfrost",    0, "mesa",  "panfrost",     "minigbm_gbm_mesa" },
            { "panthor",     0, "mesa",  "panfrost",     "minigbm_gbm_mesa" },
            { "imagination", 0, "mesa",  "powervr_mesa", "minigbm_gbm_mesa" },
            { "amdgpu",      0, "mesa",  "radeon",       "minigbm"          },
            { "raedon",      0, "mesa",  std::nullopt,   "minigbm_gbm_mesa" },
            { "virtio-gpu",  0, "mesa",  "virtio",       "minigbm",         },
            { "tegra",       0, "mesa",  std::nullopt,   "minigbm_gbm_mesa" },
            { "vmwgfx",      0, "mesa",  std::nullopt,   "minigbm",         },
            { "default",     0, "angle", "pastel",       "default",         },
        }};

        void  getKernelDriver();
        float getIntelGpuGeneration();

    public:
        std::string gpuKernelDriverName;
        GpuDriverInfo driverInfo;
        GpuUtils(const std::string &renderNode, bool swRendering);
};
