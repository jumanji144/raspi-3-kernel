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
    // configure gpio pins
    // 34 - 39 are the data bus
    for (u8 i = 34; i < 40; i++) {
        gpio::set_function(i, gpio::function::input);
    }

    // 48 - 52 is the command bus
    for (u8 i = 48; i < 53; i++) {
        gpio::set_function(i, gpio::function::alt3);
    }

    // reset host controller
    bus->control1.srst_hc = true;

    u32 timeout = 0;
    for (; timeout < max_reset_retry; timeout++) {
        if (!bus->control1.srst_hc) {
            break;
        }
        timer::wait_us(10);
    }

    uart::write("Reset host controller\n");

    if (timeout == max_reset_retry) {
        uart::write("Failed to reset host controller\n");
        return false;
    }

    this->spec = bus->slotisr_ver.sdversion;
    this->vendor = bus->slotisr_ver.vendor;

    bus->control1.clk_intlen = true; // enable internal clock
    bus->control1.data_tounit = 0b1110; // set command timeout to 2^27 cycles

    timer::wait_us(10);

    return true;
}

bool device::reset() {
    bus->interrupt_en.enable_all(); // enable all interrupts
    bus->interrupt_mask.enable_all(); // mask all interrupts
    bus->interrupt.enable_all(); // clear all interrupts

    timer::wait_us(10); // wait 10us

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
    uart::write("Sending command, CMD: {:8x}, ARG: {:8x}\n", *(u32*)&command, arg);
    bus->interrupt.enable_all(); // clear all interrupts

    bus->arg1 = arg;
    *(reg::cmdtm*)&bus->cmdtm = command;

    timer::wait_us(10);

    u32 i = 0;
    for (; i < max_command_retry; i++) {

        if (bus->interrupt.is_error()) {
            this->error = const_cast<reg::interrupt&>(bus->interrupt);
            this->success = false;
            bus->interrupt.clear_error();
            return false;
        }

        if(bus->interrupt.cmd_done) {
            break;
        }

        timer::wait_us(1);
    }

    if (i == max_command_retry) {
        uart::write("Failed to send command\n");
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
    u32 clock_div = 41666666 / freq;
    u32 shift = 32;

    u32 best_div = clock_div - 1;
    if (!best_div)
        shift = 0;
    else {
        if(!(best_div & 0xFFFF0000)) { best_div <<= 16; shift -= 16; }
        if(!(best_div & 0xFF000000)) { best_div <<= 8; shift -= 8; }
        if(!(best_div & 0xF0000000)) { best_div <<= 4; shift -= 4; }
        if(!(best_div & 0xC0000000)) { best_div <<= 2; shift -= 2; }
        if(!(best_div & 0x80000000)) { best_div <<= 1; shift -= 1; }

        if (shift > 0)
            shift -= 1;

        if (shift > 7)
            shift = 7;
    }

    u32 best = spec > 1 ? clock_div : 1 << shift;
    if (best <= 2) best = 2;

    u32 extra = spec > 1 ? (best & 0x300) >> 2 : 0;
    return ((best & 0x0FF) << 8) | extra;
}

bool device::set_clock(u32 f) {
    while(bus->status.cmd_inhibit || bus->status.dat_inhibit) { // wait for card bus to be free
        timer::wait_us(10);
    }

    bus->control1.clk_en = false; // disable clock while we set the divisor

    timer::wait_us(10);

    u32 div = get_clock_divider(f);

    uart::write("Setting clock to: {}Hz, Divisor: {}\n", f, div);

    u32* clk = (u32*)&bus->control1;
    *clk = (*clk & 0xFFFF003F) | div; // hack div into the right place

    timer::wait_us(10);

    // enable clock
    bus->control1.clk_en = true;

    // wait for clock to be stable
    u32 timeout = 0;
    for (; timeout < max_clock_retry; timeout++) {
        if (bus->control1.clk_stable) {
            break;
        }
        timer::wait_us(10);
    }

    if (timeout == max_clock_retry) {
        uart::write("Failed to set clock\n");
        return false;
    }

    return true;
}