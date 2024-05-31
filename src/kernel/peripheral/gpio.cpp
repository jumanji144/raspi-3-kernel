#include "kernel/peripheral/gpio.h"
#include "kernel/peripheral/timer.h"

void gpio::disable_pulling(u8 pin) {
    peripheral::write<u32>(gppud, 0); // disable pull up/down
    timer::wait_cycles(150);
    addr clk = pin < 32 ? gppudclk0 : gppudclk1;
    peripheral::write<u32>(clk, 1 << (pin % 32));
    timer::wait_cycles(150);
    peripheral::write<u32>(clk, 0); // flush clock
}

void gpio::pull_up(u8 pin) {
    u32 reg = pin < 32 ? gppudclk0 : gppudclk1;
    peripheral::write<u32>(gppud, 2); // enable pull up
    timer::wait_cycles(150);
    peripheral::write<u32>(reg, 1 << (pin % 32));
    timer::wait_cycles(150);
    peripheral::write<u32>(gppud, 0); // disable pull up/down
    peripheral::write<u32>(reg, 0); // flush clock
}

void gpio::pull_down(u8 pin) {
    u32 reg = pin < 32 ? gppudclk0 : gppudclk1;
    peripheral::write<u32>(gppud, 1); // enable pull down
    timer::wait_cycles(150);
    peripheral::write<u32>(reg, 1 << (pin % 32));
    timer::wait_cycles(150);
    peripheral::write<u32>(gppud, 0); // disable pull up/down
    peripheral::write<u32>(reg, 0); // flush clock
}

void gpio::set_function(u8 pin, gpio::function func) {
    u32 reg = gpfsel0 + (pin / 10) * 4;
    u8 shift = (pin % 10) * 3;
    u32 mask = 0b111 << shift;
    u32 value = static_cast<u32>(func) << shift;
    u32 current = peripheral::read<u32>(reg);
    peripheral::write<u32>(reg, (current & ~mask) | value);
}

void gpio::enable_interrupt(u8 pin) {
    u32 reg = pin < 32 ? gphen0 : gphen1;
    peripheral::write<u32>(reg, 1 << (pin % 32));
}