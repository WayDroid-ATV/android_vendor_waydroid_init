/*
 * Copyright (C) 2021 The LineageOS Project
 * Copyright (C) 2026 The WayDroid-ATV Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <string>
#include <cstdint>

namespace DalvikHeap {
    struct dalvikHeapInfo {
        uint64_t    minMemSize;
        std::string heapStartSize;
        std::string heapGrowthLimit;
        std::string heapSize;
        std::string heapMinFree;
        std::string heapMaxFree;
        std::string heapTargetUtilization;
    };

    void setDalvikHeap();
}
