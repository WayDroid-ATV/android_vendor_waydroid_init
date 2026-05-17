/*
 * Copyright (C) 2021 The LineageOS Project
 * Copyright (C) 2026 The WayDroid-ATV Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string>
#include <array>
#include <cstring>
#include <cerrno>
#include <sys/sysinfo.h>
#include <android-base/properties.h>

#include "log.h"
#include "dalvik_heap.h"

#define HEAPSTARTSIZE_PROP "dalvik.vm.heapstartsize"
#define HEAPGROWTHLIMIT_PROP "dalvik.vm.heapgrowthlimit"
#define HEAPSIZE_PROP "dalvik.vm.heapsize"
#define HEAPMINFREE_PROP "dalvik.vm.heapminfree"
#define HEAPMAXFREE_PROP "dalvik.vm.heapmaxfree"
#define HEAPTARGETUTILIZATION_PROP "dalvik.vm.heaptargetutilization"
#define LOWRAM_PROP "ro.config.low_ram"

#define GB(b) static_cast<uint64_t>(b * 1024ull * 1024 * 1024)

namespace DalvikHeap {
    static constexpr std::array<dalvikHeapInfo, 7> dalvikHeapConfigs = {{
        {
            .minMemSize            = GB(15),
            .heapStartSize         = "32m",
            .heapGrowthLimit       = "448m",
            .heapSize              = "640m",
            .heapMinFree           = "16m",
            .heapMaxFree           = "64m",
            .heapTargetUtilization = "0.4",
        },
        {
            .minMemSize            = GB(11),
            .heapStartSize         = "24m",
            .heapGrowthLimit       = "384m",
            .heapSize              = "512m",
            .heapMinFree           = "8m",
            .heapMaxFree           = "56m",
            .heapTargetUtilization = "0.42",
        },
        {
            .minMemSize            = GB(7),
            .heapStartSize         = "24m",
            .heapGrowthLimit       = "256m",
            .heapSize              = "512m",
            .heapMinFree           = "8m",
            .heapMaxFree           = "48m",
            .heapTargetUtilization = "0.46",
        },
        {
            .minMemSize            = GB(5),
            .heapStartSize         = "16m",
            .heapGrowthLimit       = "256m",
            .heapSize              = "512m",
            .heapMinFree           = "8m",
            .heapMaxFree           = "32m",
            .heapTargetUtilization = "0.5",
        },
        {
            .minMemSize            = GB(3),
            .heapStartSize         = "8m",
            .heapGrowthLimit       = "256m",
            .heapSize              = "512m",
            .heapMinFree           = "8m",
            .heapMaxFree           = "16m",
            .heapTargetUtilization = "0.6",
        },
        {
            .minMemSize            = GB(1.3),
            .heapStartSize         = "8m",
            .heapGrowthLimit       = "128m",
            .heapSize              = "256m",
            .heapMinFree           = "512k",
            .heapMaxFree           = "8m",
            .heapTargetUtilization = "0.75",
        },
        {
            .minMemSize            = 0,
            .heapStartSize         = "8m",
            .heapGrowthLimit       = "96m",
            .heapSize              = "256m",
            .heapMinFree           = "512k",
            .heapMaxFree           = "8m",
            .heapTargetUtilization = "0.75",
        },
    }};

    void setDalvikHeap() {
        const dalvikHeapInfo *dalvikHeapInfoPtr = &dalvikHeapConfigs[6];
        struct sysinfo sysInfo;

        if (sysinfo(&sysInfo) == 0) {
            for (const auto &heapInfo : dalvikHeapConfigs) {
                if (sysInfo.totalram * sysInfo.mem_unit > heapInfo.minMemSize) {
                    dalvikHeapInfoPtr = &heapInfo;
                    break;
                }
            }
        } else {
            Log::warn("sysinfo() failed: {}", strerror(errno));
        }

        // Set low RAM prop if system RAM is less than 4GB (available RAM is less than 3GB)
        if (dalvikHeapInfoPtr->minMemSize < GB(3)) {
            Log::info("Low RAM mode enabled");
            android::base::SetProperty(LOWRAM_PROP, "true");
        }

        android::base::SetProperty(HEAPSTARTSIZE_PROP, dalvikHeapInfoPtr->heapStartSize);
        android::base::SetProperty(HEAPGROWTHLIMIT_PROP, dalvikHeapInfoPtr->heapGrowthLimit);
        android::base::SetProperty(HEAPSIZE_PROP, dalvikHeapInfoPtr->heapSize);
        android::base::SetProperty(HEAPTARGETUTILIZATION_PROP, dalvikHeapInfoPtr->heapTargetUtilization);
        android::base::SetProperty(HEAPMINFREE_PROP, dalvikHeapInfoPtr->heapMinFree);
        android::base::SetProperty(HEAPMAXFREE_PROP, dalvikHeapInfoPtr->heapMaxFree);
    }
}
