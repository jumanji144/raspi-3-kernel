#pragma once
#include "../../common.h"

namespace peripheral {
    static constexpr addr base = 0x3f000000;

    static constexpr addr system_timer = base + 0x3000;
    static constexpr addr interrupts = base + 0xb000;
    static constexpr addr mailbox = base + 0xb880;
    static constexpr addr uart0 = base + 0x201000;
    static constexpr addr uart1 = base + 0x215000;
    static constexpr addr gpio = base + 0x200000;
    static constexpr addr emmc = base + 0x300000;

    template<typename T>
    static inline T read(addr reg) {
        return *(T*)reg;
    }

    template<typename T>
    static inline void write(addr reg, T data) {
        *(T*)reg = data;
    }

};