#pragma once
#include "../../common.h"

namespace peripheral {
    inline constexpr addr base = 0x3f000000;

    inline constexpr addr system_timer = base + 0x3000;
    inline constexpr addr interrupts = base + 0xb000;
    inline constexpr addr mailbox = base + 0xb880;
    inline constexpr addr rand = base + 0x104000;
    inline constexpr addr uart0 = base + 0x201000;
    inline constexpr addr uart1 = base + 0x215000;
    inline constexpr addr gpio = base + 0x200000;
    inline constexpr addr emmc = base + 0x300000;

    template<typename T>
    static inline T read(addr reg) {
        return *(T*)reg;
    }

    template<typename T>
    static inline void write(addr reg, T data) {
        *(T*)reg = data;
    }

};