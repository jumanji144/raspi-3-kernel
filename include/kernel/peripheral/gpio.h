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

}