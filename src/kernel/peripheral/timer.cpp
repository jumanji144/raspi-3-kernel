#include "kernel/peripheral/timer.h"
#include "kernel/asm.h"

u64 timer::get_system_timer() {
    u32 hi = peripheral::read<u32>(timer_hi);
    u32 lo = peripheral::read<u32>(timer_lo);

    // recheck if the high bits have changed
    if (hi != peripheral::read<u32>(timer_hi)) {
        hi = peripheral::read<u32>(timer_hi);
        lo = peripheral::read<u32>(timer_lo);
    }

    return ((u64)hi << 32) | lo;
}

u64 timer::get_counter_frequency() {
    return cntfrq_el0();
}

u64 timer::get_current_count() {
    return cntpct_el0();
}

void timer::wait_us(u64 us) {
    u64 freq = cntfrq_el0();
    u64 start = cntpct_el0();

    u64 required = ((freq / 1000) * us) / 1000;

    while (cntpct_el0() - start < required) { }
}

void timer::wait_ms(u64 ms) {
    wait_us(ms * 1000);
}

void timer::wait_cycles(u64 cycles) {
    while (cycles--) { nop(); }
}