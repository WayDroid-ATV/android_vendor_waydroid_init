/*
 * Copyright (C) 2026 The WayDroid-ATV Project
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <string>
#include <fstream>
#include <cstdint>

#define IPCONFIG_APEX_TXT "/data/misc/apexdata/com.android.tethering/misc/ethernet/ipconfig.txt"
#define IPCONFIG_TXT "/data/misc/ethernet/ipconfig.txt"

class IPConfig {
    private:
        std::string IPv4ToString(struct in_addr inAddr) const;
        void writeToConfig(uint32_t data);
        void writeToConfig(const std::string &data);

    public:
        inline static std::string dnsServer   = "8.8.8.8";
        inline static std::string gatewayAddr = "172.16.0.1";
        inline static std::string ipAddr      = "172.16.0.2";
        inline static uint8_t     ipMask      = 16;

        std::ofstream config;

        IPConfig();
        static bool generate();
};
