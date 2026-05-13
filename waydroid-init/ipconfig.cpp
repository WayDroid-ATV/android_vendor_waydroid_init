/*
 * Copyright (C) 2026 The WayDroid-ATV Project
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * ipconfig.cpp: Based on Redroid's ipconfigstore, with C++ refactors
 */

#include <string>
#include <sstream>
#include <system_error>
#include <fstream>
#include <filesystem>
#include <bit>
#include <iomanip>
#include <cstdint>
#include <cstring>
#include <cerrno>
#include <endian.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>

#include "log.h"
#include "utils.h"
#include "android-uid.h"
#include "ipconfig.h"

IPConfig::IPConfig() {
    std::ifstream resolvConf("/dev/.docker/resolv.conf"),
                  routeTable("/proc/net/route");
    std::string   line;

    // Get IPv4 address/netmask using getifaddrs()
    {
        struct ifaddrs     *ifAddrStruct;
        struct sockaddr_in *ipAddr,
                           *maskAddr;

        if (getifaddrs(&ifAddrStruct) == 0) {
            for (auto addr = ifAddrStruct; addr != NULL; addr = addr->ifa_next) {
                if (addr->ifa_addr->sa_family != AF_INET || strcmp(addr->ifa_name, "eth0") != 0) {
                    continue;
                }

                ipAddr   = (struct sockaddr_in *) addr->ifa_addr;
                maskAddr = (struct sockaddr_in *) addr->ifa_netmask;

                if (ipAddr == NULL || maskAddr == NULL) continue;

                this->ipAddr = IPv4ToString(ipAddr->sin_addr);
                this->ipMask = std::popcount(maskAddr->sin_addr.s_addr);
                break;
            }

            freeifaddrs(ifAddrStruct);
        } else {
            Log::warn("getifaddrs() failed: {}", strerror(errno));
        }
    }

    // Extract default gateway from route table
    if (routeTable.is_open()) {
        std::string line, iface;
        uint32_t    destAddr, gatewayAddr;

        // Skip title line
        std::getline(routeTable, line);

        while (std::getline(routeTable, line)) {
            std::stringstream ss(line);
            ss >> iface >> std::hex >> destAddr >> gatewayAddr;

            if (destAddr == 0) {
                this->gatewayAddr = IPv4ToString((struct in_addr) { .s_addr = gatewayAddr });
                break;
            }
        }
    } else {
        Log::warn("Failed to read /proc/net/route: {}", strerror(errno));
    }

    // Extract DNS from resolv.conf
    if (resolvConf.is_open()) {
        std::string key, value;

        while (std::getline(resolvConf, line)) {
            std::stringstream ss(line);
            ss >> key >> value;

            if (key == "nameserver") {
                this->dnsServer = value;
                break;
            }
        }
    } else {
        Log::warn("Failed to read resolv.conf: {}", strerror(errno));
    }

    config.open(IPCONFIG_TXT, std::ios::out | std::ios::binary);

    if (!config.is_open()) {
        Log::err("Failed to open {}: {}", IPCONFIG_TXT, strerror(errno));
    }
}

std::string IPConfig::IPv4ToString(struct in_addr inAddr) const {
    char result[INET_ADDRSTRLEN] = {'\0'};
    inet_ntop(AF_INET, &inAddr, result, INET_ADDRSTRLEN);
    return std::string(result);
}

void IPConfig::writeToConfig(uint32_t data) {
    uint32_t packedData = htobe32(data);
    config.write(reinterpret_cast<const char*>(&packedData), sizeof(packedData));
}

void IPConfig::writeToConfig(const std::string &data) {
    uint16_t packedLength = htobe16(data.length());
    config.write(reinterpret_cast<const char*>(&packedLength), sizeof(packedLength));
    config.write(data.c_str(), data.length());
}

bool IPConfig::generate() {
    IPConfig        ipconfig;
    std::error_code error;

    if (!ipconfig.config.is_open()) return false;

    // Remove apex config if exist
    if (std::filesystem::exists(IPCONFIG_APEX_TXT)) {
        std::filesystem::remove_all(IPCONFIG_APEX_TXT, error);

        if (error.value()) {
            Log::warn("remove() failed for " IPCONFIG_APEX_TXT ": {}", error.message());
        }
    }

    // Generate ipconfig.txt
    ipconfig.writeToConfig(3); // ipconfig.txt version 3

    ipconfig.writeToConfig("ipAssignment");
    ipconfig.writeToConfig("STATIC");

    ipconfig.writeToConfig("linkAddress");
    ipconfig.writeToConfig(ipAddr);
    ipconfig.writeToConfig(ipMask);

    ipconfig.writeToConfig("gateway");
    ipconfig.writeToConfig(1); // Default route (dest)
    ipconfig.writeToConfig("0.0.0.0");
    ipconfig.writeToConfig(0);
    ipconfig.writeToConfig(1); // Have a gateway.
    ipconfig.writeToConfig(gatewayAddr);

    ipconfig.writeToConfig("dns");
    ipconfig.writeToConfig(dnsServer);

    ipconfig.writeToConfig("proxySettings");
    ipconfig.writeToConfig("NONE");

    ipconfig.writeToConfig("id");
    ipconfig.writeToConfig("eth0");

    ipconfig.writeToConfig("eos");
    ipconfig.config.close();

    // Setup file permission
    if (!Utils::setFilePermission(IPCONFIG_TXT, AID_SYSTEM, AID_SYSTEM, 0770)) return false;

    return true;
}
