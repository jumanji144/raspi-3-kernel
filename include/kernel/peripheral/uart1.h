#pragma once
#include "common.h"
#include "peripheral.h"
#include "gpio.h"

namespace uart {

    enum {
        base = peripheral::uart1,
        aux_enable = base + 0x04,
        aux_mu_io_reg = base + 0x40,
        aux_mu_ier_reg = base + 0x44,
        aux_mu_iir_reg = base + 0x48,
        aux_mu_lcr_reg = base + 0x4C,
        aux_mu_mcr_reg = base + 0x50,
        aux_mu_lsr_reg = base + 0x54,
        aux_mu_msr_reg = base + 0x58,
        aux_mu_scratch = base + 0x5C,
        aux_mu_cntl_reg = base + 0x60,
        aux_mu_stat_reg = base + 0x64,
        aux_mu_baud_reg = base + 0x68
    };

    void init();
    void write(char c);
    void write(const char* str);
    void write(uint64_t value, u8 base = 10, u8 width = 0);
    char read();

}