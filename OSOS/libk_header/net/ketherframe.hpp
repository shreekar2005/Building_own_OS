#ifndef ETHERFRAME_HPP
#define ETHERFRAME_HPP

#include "essential/ktypes.hpp"

namespace net {

    struct EtherFrameHeader {
        uint8_t dstMAC[6];
        uint8_t srcMAC[6];
        uint16_t etherType;
    } __attribute__((packed));

    // Common EtherTypes
    // 0x0800 = IPv4
    // 0x0806 = ARP
    // 0x86DD = IPv6

    // Helper to switch between Little Endian (x86) and Big Endian (Network)
    // Example: 0x0800 becomes 0x0008
    static inline uint16_t htons(uint16_t v) {
        return (v << 8) | (v >> 8);
    }
}

#endif