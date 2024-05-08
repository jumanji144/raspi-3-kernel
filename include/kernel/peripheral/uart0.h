#pragma once
#include "common.h"
#include "peripheral.h"
#include "mailbox/mailbox.h"
#include "gpio.h"

namespace uart {

    union flags {
        struct {
            // taken from: https://www.raspberrypi.org/documentation/hardware/raspberrypi/bcm2711/rpi_DATA_2711_1p0.pdf
            u8 clear_to_send : 1;
            u8 data_set_ready : 1;
            u8 data_carrier_detect : 1;
            u8 busy : 1;
            u8 receive_queue_empty : 1;
            u8 transmit_queue_full : 1;
            u8 receive_queue_full : 1;
            u8 transmit_queue_empty : 1;
            u8 ring_indicator : 1;
            u32 reserved : 23;
        };
        u32 raw;
    };

    union control {
        struct {
            u8 uart_enabled : 1;
            u8 sir_enabled : 1;
            u8 sir_low_power_mode : 1;
            u8 reserved1 : 4;
            u8 loop_back_enabled : 1;
            u8 transmit_enabled : 1;
            u8 receive_enabled : 1;
            u8 data_transmit_ready : 1;
            u8 request_to_send : 1;
            u8 data1 : 1;
            u8 data2 : 1;
            u8 rtse : 1;
            u8 ctse : 1;
            u8 padding : 2;
        };
        u32 raw;
    };

    enum {
        base = peripheral::uart0,
        data = base + 0x00,
        receive_status = base + 0x04,
        transmit_status = base + 0x18,
        integer_baud_rate_divisor = base + 0x24,
        fractional_baud_rate_divisor = base + 0x28,
        line_control = base + 0x2C,
        control_register = base + 0x30,
        interrupt_fifo_level_select = base + 0x34,
        interrupt_mask_set_clear = base + 0x38,
        raw_interrupt_status = base + 0x3C,
        masked_interrupt_status = base + 0x40,
        interrupt_clear = base + 0x44,
        dma_control = base + 0x48,
        interrupt_clear_register = base + 0x80,
        interrupt_pending_register = base + 0x84,
        interrupt_overrun_register = base + 0x88,
        transmit_fifo = base + 0x8C
    };

    void init();
    flags read_flags();
    void write(char c);
    void write(const char* str);
    char read();

}