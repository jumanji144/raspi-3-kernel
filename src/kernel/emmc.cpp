#include <kernel/peripheral/emmc.h>
#include <kernel/peripheral/gpio.h>
#include "kernel/peripheral/timer.h"
#include "kernel/peripheral/uart1.h"

inline constexpr u32 max_reset_retry = 100;
inline constexpr u32 max_command_retry = 100;
inline constexpr u32 max_data_retry = 100;
inline constexpr u32 max_clock_retry = 10000;

using namespace emmc;

bool device::init() {

    // disable internal clock
    bus->control1.clk_en = false;
    bus->control1.clk_intlen = false;

    bus->control1.srst_hc = true;

    if(!timeout_wait([] { return !bus->control1.srst_hc; }, max_reset_retry)) {
        uart::write("EMMC: device::init: Failed to reset host controller\n");
        return false;
    }

    uart::write("EMMC: device::init: Reset host controller\n");

    this->spec = bus->slotisr_ver.sdversion;
    this->vendor = bus->slotisr_ver.vendor;

    this->capabilities1 = bus->capabilities1;
    this->capabilities2 = bus->capabilities2;

    bus->control2.raw = 0x0;

    bus->control1.clk_intlen = true;
    bus->control1.data_tounit = 0x7;

    return true;
}

bool device::reset() {
    bus->interrupt_en.disable_all(); // disable all interrupts, 0 = enabled
    bus->interrupt_mask.enable_all(); // mask all interrupts
    bus->interrupt.enable_all(); // clear all interrupts

    timer::wait_us(2000);

    return true;
}

bool device::reset_command() {
    bus->control1.srst_cmd = true;

    for (u32 i = 0; i < max_reset_retry; i++) {
        if (!bus->control1.srst_cmd) {
            return true;
        }
        timer::wait_us(10);
    }

    return false;
}

bool device::send_command(reg::cmdtm command, u32 arg) {
    // wait for command inhibit to be clear
    while(bus->status.cmd_inhibit) {
        timer::wait_us(10);
    }

    if(command.rspns_type == reg::rt48busy && command.type == reg::abort) {
        // wait for data inhibit to be clear
        while(bus->status.dat_inhibit) {
            timer::wait_us(10);
        }
    }

    uart::write("EMMC: device::send_command: Sending command, CMD: {:8x}, ARG: {:8x}\n", *(u32*)&command, arg);
    bus->interrupt.enable_all(); // clear all interrupts

    bus->arg1 = arg;
    *(reg::cmdtm*)&bus->cmdtm = command;

    timer::wait_us(2000);

    u32 i = 0;
    for (; i < max_command_retry; i++) {

        if (bus->interrupt.is_error()) {
            this->error = const_cast<reg::interrupt&>(bus->interrupt);
            this->success = false;
            bus->interrupt.clear_error();
            uart::write("EMMC: device::send_command: Error interrupt\n");
            return false;
        }

        if(bus->interrupt.cmd_done) {
            break;
        }

        timer::wait_us(10);
    }

    if (i == max_command_retry) {
        uart::write("EMMC: device::send_command: Timeout waiting for command to complete\n");
        this->success = false;
        return false;
    }

    switch (command.rspns_type) {
        case reg::rt48:
        case reg::rt48busy:
            this->response[0] = bus->resp[0];
            break;
        case reg::rt136:
            this->response[0] = bus->resp[0];
            this->response[1] = bus->resp[1];
            this->response[2] = bus->resp[2];
            this->response[3] = bus->resp[3];
            break;
        case reg::none:
            break;
    }

    // do data transfer here
    if (command.isdata) {
        if(!data_transfer(command))
            return false;
    }

    this->success = true;
    return true;
}

bool device::transfer_block(bool write, u32* buf) {
    u32 timeout = 0;
    for (; timeout < max_data_retry; timeout++) {
        bool ready = write ? bus->interrupt.write_ready : bus->interrupt.read_ready;
        if (ready) break;

        if (bus->interrupt.is_error()) {
            bus->interrupt.clear_error();
            this->error = const_cast<reg::interrupt&>(bus->interrupt);
            this->success = false;
            return false;
        }
    }

    if (timeout == max_data_retry) {
        this->success = false;
        return false;
    }

    u32 length = this->block_size / sizeof(u32);

    // write data
    if (write) {
        for (u32 i = 0; i < length; i++) {
            bus->data = buf[i];
        }
    } else {
        for (u32 i = 0; i < length; i++) {
            buf[i] = bus->data;
        }
    }

    return true;
}

bool device::data_transfer(reg::cmdtm command) {

    bool write = command.dir == reg::data_dir::host_to_card;

    if (write) {
        bus->interrupt_en.write_ready = true;
    } else {
        bus->interrupt_en.read_ready = true;
    }

    u32* dat = buffer;
    for(u32 block = 0; block < this->blocks; block++) {
        if (!transfer_block(write, dat)) {
            return false;
        }
        buffer += this->block_size;
    }

    return true;
}

u32 device::get_clock_divider(u32 freq) const {
    u32 target = 0;
    if (freq > base_clock)
        target = 1;
    else {
        target = base_clock / freq;
        if (base_clock % freq)
            target--; // round down
    }

    u32 div = -1;
    for (u8 bit = 31; bit >= 0; bit--) {
        u32 mask = 1 << bit;
        if (target & mask) {
            div = bit;
            target &= ~mask;
            if (target) {
                div++; // round up
            }
            break;
        }
    }

    if (div == -1) // was unable to find a divisor
        div = 31;
    if (div >= 32)
        div = 31;

    if (div != 0)
        div = 1 << (div - 1);

    if (div > 0x3FF)
        div = 0x3FF;

    return div;
}

bool device::set_clock(u32 f) {
    while(bus->status.cmd_inhibit || bus->status.dat_inhibit) { // wait for card bus to be free
        timer::wait_us(10);
    }

    bus->control1.clk_en = false; // disable clock while we set the divisor

    timer::wait_us(100);

    u32 div = get_clock_divider(f);
    u32 val = ((div & 0xff) << 8) | (((div >> 8) & 0x3) << 6) | (0 << 5);

    u32 dem = 1;
    if (div != 0)
        dem = 2 * div;
    u32 actual = base_clock / dem;

    uart::write("Setting clock to: {}Hz, div: {}, actual: {}Hz\n", f, div, actual);

    bus->control1.raw = (bus->control1.raw & 0xFFFF003F) | val;

    timer::wait_us(10);

    // enable clock
    bus->control1.clk_en = true;

    timer::wait_us(10);

    // wait for clock to be stable
    u32 timeout = 0;
    for (; timeout < max_clock_retry; timeout++) {
        if (bus->control1.clk_stable) {
            break;
        }

        timer::wait_us(10);
    }

    if (timeout == max_clock_retry) {
        uart::write("Clock not stable within timeout\n");
        return false;
    }

    return true;
}