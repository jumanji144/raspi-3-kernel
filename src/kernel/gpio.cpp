#include <kernel/peripheral/gpio.h>
#include <kernel/peripheral/timer.h>

void gpio::disable_pulling(u8 pin) {
    peripheral::write<u32>(gppud, 0); // disable pull up/down
    timer::wait_cycles(150);
    addr clk = pin < 32 ? gppudclk0 : gppudclk1;
    peripheral::write<u32>(clk, 1 << (pin % 32));
    timer::wait_cycles(150);
    peripheral::write<u32>(clk, 0); // flush clock
}

void gpio::pull_up(u8 pin) {
    disable_pulling(pin);
    peripheral::write<u32>(gppud, 2); // enable pull up
    timer::wait_cycles(150);
    peripheral::write<u32>(gppudclk0, 1 << (pin % 32));
    timer::wait_cycles(150);
    peripheral::write<u32>(gppudclk0, 0); // flush clock
}

void gpio::pull_down(u8 pin) {
    disable_pulling(pin);
    peripheral::write<u32>(gppud, 1); // enable pull down
    timer::wait_cycles(150);
    peripheral::write<u32>(gppudclk0, 1 << (pin % 32));
    timer::wait_cycles(150);
    peripheral::write<u32>(gppudclk0, 0); // flush clock
}

void gpio::set_function(u8 pin, gpio::function func) {
    u8 reg = pin / 10;
    u8 shift = (pin % 10) * 3;

    u32 sel = peripheral::read<u32>(gpfsel0 + reg * 4);
    sel &= ~(7 << shift);
    sel |= func << shift;

    peripheral::write<u32>(gpfsel0 + reg * 4, sel);
}