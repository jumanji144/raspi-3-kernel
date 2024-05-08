#include "kernel/peripheral/uart1.h"
#include "common/string.h"

static inline void delay(int32_t count)
{
    asm volatile("__delay_%=: subs %[count], %[count], #1\n bne __delay_%=\n"
            : "=r"(count): [count]"0"(count) : "cc");
}

void uart::init() {
    peripheral::write<u32>(uart::aux_enable, 1);
    peripheral::write<u32>(uart::aux_mu_cntl_reg, 0);
    peripheral::write<u32>(uart::aux_mu_lcr_reg, 3);
    peripheral::write<u32>(uart::aux_mu_mcr_reg, 0);
    peripheral::write<u32>(uart::aux_mu_ier_reg, 0);
    peripheral::write<u32>(uart::aux_mu_iir_reg, 0xC6);
    peripheral::write<u32>(uart::aux_mu_baud_reg, 270); // 115200 baud

    u32 ra = peripheral::read<u32>(gpio::gpfsel1);
    ra &= ! ((7 << 12) | (7 << 15)); // gpio14 and gpio15
    ra |= (2 << 12) | (2 << 15); // alt5

    peripheral::write<u32>(gpio::gpfsel1, ra);
    // disable pull-up/pull-down
    peripheral::write<u32>(gpio::gppud, 0);

    // wait 150 cycles
    delay(150);

    // disable pull-up/pull-down for pins 14 and 15
    peripheral::write<u32>(gpio::gppudclk0, (1 << 14) | (1 << 15));

    // wait 150 cycles
    delay(150);

    // write 0 to gppudclk0 to make it take effect
    peripheral::write<u32>(gpio::gppudclk0, 0);

    peripheral::write<u32>(uart::aux_mu_cntl_reg, 3);
}

char uart::read() {
    constexpr char ready = 0x01;
    while (! (peripheral::read<u32>(uart::aux_mu_lsr_reg) & ready)) { }
    char c = peripheral::read<u32>(uart::aux_mu_io_reg);
    return c == '\r' ? '\n' : c;
}

void uart::write(char c) {
    constexpr char ready = 0x20;
    while (! (peripheral::read<u32>(uart::aux_mu_lsr_reg) & ready)) { }
    peripheral::write<u32>(uart::aux_mu_io_reg, c);
    if (c == '\n') {
        write('\r');
    }
}

void uart::write(const char* str) {
    while (*str) {
        write(*str++);
    }
}

void uart::write(uint64_t value, u8 base, u8 width) {
    char buffer[16];
    str::itos(value, buffer, base, width);
    write(buffer);
}