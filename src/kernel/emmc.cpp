#include <kernel/peripheral/emmc.h>
#include <kernel/peripheral/gpio.h>
#include "kernel/peripheral/timer.h"
#include "kernel/peripheral/uart1.h"
#include <common/memory/mem.h>

bool emmc::device::init() {
    host_version = slotisr_ver::get(bus->slotisr_ver).sdversion;
    base_clock_rate = this->get_base_clock_rate();

    if (base_clock_rate == 0) {
        uart::write("EMMC: failed to get base clock rate\n");
        return false;
    }

    uart::write("EMMC: host version v{}\n", host_version + 1);
    uart::write("EMMC: base clock rate {}MHz\n", base_clock_rate / 1000000);

    // Reset the card.
    bus->control0 = 0;
    bus->control1 |= control1::srst_hc;

    u32 tout = 0;
    for (; tout < 10000; ++tout) {
        timer::wait_us(10);
        if (!(bus->control1 & control1::srst_hc)) {
            break;
        }
    }

    if (tout == 10000) {
        uart::write("EMMC: failed to reset\n");
        return false;
    }

    uart::write("EMMC: reset OK\n");

    // enable internal clock and set the timeout to 1110, TMCLK * 2^(27), approx 2 seconds
    bus->control1 |= ( control1::clk_intlen | control1::tounit(0xe) );

    timer::wait_us(10);
    return true;
}

bool emmc::device::send_command(reg::cmdtm command, u32 arg) {
    // wait for command inhibit
    while (bus->status & status::cmd_inhibit) {
        timer::wait_us(10);
    }

    // convert command to a 32-bit integer
    u32 code = *reinterpret_cast<u32*>(&command);

    uart::write("EMMC: sending command {:8x} arg {:8x}\n", code, arg);

    // set the transfer blocks
    bus->blksizecnt = block_size | (blocks_to_transfer << 16);

    bus->interrupt = bus->interrupt; // clear interrupts
    bus->arg1 = arg;
    bus->cmdtm = code;

    timer::wait_us(1000);

    // wait for cmd done interrupt
    if(!wait_for_interrupt(interrupt::cmd_done)) {
        return false;
    }

    switch (command.rspns_type) {
        case reg::rt48:
        case reg::rt48busy:
            response[0] = bus->resp[0];
            break;
        case reg::rt136:
            response[0] = bus->resp[0];
            response[1] = bus->resp[1];
            response[2] = bus->resp[2];
            response[3] = bus->resp[3];
            break;
        default:
            break;
    }

    if (command.isdata) {
        return do_data_transfer(command.dir == reg::data_dir::host_to_card);
    }

    return true;
}

bool emmc::device::reset_command() {
    bus->control1 |= control1::srst_cmd;
    u32 tout = 0;
    for (; tout < 10000; ++tout) {
        timer::wait_us(10);
        if (!(bus->control1 & control1::srst_cmd)) {
            break;
        }
    }

    return tout != 10000;
}

bool emmc::device::do_data_transfer(bool write) {
    u32 wait_mask = write ? interrupt::write_ready : interrupt::read_ready;

    for (int blk = 0; blk < this->blocks_to_transfer; blk++) {
        // don't clear bit so we can re-use this function to check for errors
        if (!wait_for_interrupt(wait_mask, false)) {
            return false;
        }

        u32 length = this->block_size;
        if (write) {
            for (; length > 0; length -= 4) {
                bus->data = *this->buffer++;
            }
        } else {
            for (; length > 0; length -= 4) {
                *this->buffer++ = bus->data;
            }
        }
    }

    return true;
}

bool emmc::device::wait_for_interrupt(u32 mask, bool clear, u32 timeout) {
    u32 all_mask = mask | interrupt::error_mask;
    last_timeout = false;
    last_error = {};
    u32 tout = 0;
    for (; tout < timeout; ++tout) {
        u32 ints = bus->interrupt;
        if (ints & all_mask) {
            break;
        }
        timer::wait_us(10);
    }
    if (tout == timeout) {
        last_timeout = true;
        return false;
    }

    if (clear)
        bus->interrupt = all_mask; // clear interrupts

    auto itrp = interrupt::get(bus->interrupt);
    if (itrp.is_error()) {
        this->last_error = itrp;
        // if we have an error clear it
        bus->interrupt = all_mask;
        return false;
    }

    return true;
}