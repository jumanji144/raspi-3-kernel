#pragma once
#include <common.h>
#include "peripheral.h"

namespace gpio {

    static constexpr addr gpfsel0 = peripheral::gpio + 0x00;
    static constexpr addr gpfsel1 = peripheral::gpio + 0x04;
    static constexpr addr gpset0 = peripheral::gpio + 0x1C;
    static constexpr addr gpclr0 = peripheral::gpio + 0x28;
    static constexpr addr gppud = peripheral::gpio + 0x94;
    static constexpr addr gppudclk0 = peripheral::gpio + 0x98;
    static constexpr addr gppudclk1 = peripheral::gpio + 0x9C;

    enum function {
        input = 0,
        output = 1,
        alt0 = 4,
        alt1 = 5,
        alt2 = 6,
        alt3 = 7,
        alt4 = 3,
        alt5 = 2
    };

    void disable_pulling(u8 pin);
    void pull_up(u8 pin);
    void pull_down(u8 pin);
    void set_function(u8 pin, function func);

}