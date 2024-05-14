#pragma once
#include <common.h>
#include <kernel/peripheral/peripheral.h>

namespace timer {

    inline constexpr addr timer_lo = peripheral::system_timer + 0x04;
    inline constexpr addr timer_hi = peripheral::system_timer + 0x08;

    u64 get_system_timer();

    u64 get_current_count();
    u64 get_counter_frequency();

    void wait_ms(u64 ms);
    void wait_us(u64 us);

    void wait_cycles(u64 cycles);

    // conversion operators


}