#include <kernel/peripheral/sd.h>
#include <kernel/peripheral/timer.h>
#include <kernel/peripheral/gpio.h>
#include <kernel/peripheral/emmc.h>
#include <kernel/peripheral/uart1.h>
#include <kernel/mailbox/mailbox.h>

bool emmc::sd::init() {
    /**
     * GPIO OVERVIEW
     * PIN | PULL | ALT3
     * 48: | HIGH | SD1_CLK
     * 49: | HIGH | SD1_CMD
     * 50: | HIGH | SD1_DAT0
     * 51: | HIGH | SD1_DAT1
     * 52: | HIGH | SD1_DAT2
     * 53: | HIGH | SD1_DAT3
     */
    for (int i = 0; i < 6; i++) {
        gpio::set_function(48 + i, gpio::function::alt3);
        gpio::pull_up(48 + i);
    }

    uart::write("EMMC: GPIO set up\n");

    device::init();

    // set the clock rate to 400 kHz
    if (!this->set_clock_rate(clock_rate_identification)) {
        uart::write("EMMC: failed to set clock rate\n");
        return false;
    }

    // enable all interrupt lines to the arm processor
    bus->interrupt_en = 0xffffffff;
    // mask enable all interrupts
    bus->interrupt_mask = 0xffffffff;

    // sd initialization process

    // tell sdcard to go into IDLE state, this marks the beginning of the initialization process
    if(!send_command(cmd::go_idle_state, 0)) {
        uart::write("EMMC: failed to send go idle state\n");
        return false;
    }

    // next we must negotiate interface conditions with the sdcard, this is for legacy reasons, aka v1 cards
    constexpr u32 check_pattern = 0b10101010;
    u32 if_cond = 0x1 << 8 | check_pattern; // 2.7-3.6V range and check pattern
    if (!send_command(cmd::send_if_cond, if_cond)) {
        // v1 cards do not respond to this command, but don't throw an error
        if (last_error.is_error() && !(last_timeout || last_error.is_timeout())) {
            uart::write("EMMC: failed to send if cond\n");
            return false;
        } else {
            // we had a timeout, which is ok. we just need to flush the command registers
            if (!reset_command())
                return false;
        }
    }

    if (response[0] != if_cond) {
        uart::write("EMMC: invalid if cond response\n");
        return false;
    }

    // now that we established that the card may interface with us, we must negotiate the operating conditions
    u32 ocr = ocr::base;
    // build flags
    if (host_version > host_spec::v2) {
        // inquiring about sdhc, sdxc and low voltage support
        ocr |= ( ocr::sdhc | ocr::sdxc | ocr::voltage_1_8 );
    }

    // now we repeatedly call the send op command, until the card says it is done sending operating conditions
    bool card_busy = true;
    while (card_busy) {
        if (!send_app_command(cmd::send_op_cond, ocr)) {
            uart::write("EMMC: failed to send op cond\n");
            return false;
        }
        // if the top of the response is set, then the card is done sending
        if ((response[0] >> 31) == 1) {
            card_busy = false;
        }
        timer::wait_us(400);
    }

    // now our response should contain all the supported features of the card
    sdhc = response[0] & ocr::sdhc;
    sdxc = response[0] & ocr::sdxc;
    voltage_1_8 = response[0] & ocr::voltage_1_8;

    uart::write("EMMC: sd card supports SDHC: {}, SDXC: {}, 1.8V: {}\n", sdhc, sdxc, voltage_1_8);

    // TODO: if the sd card supports 1.8V, then we should switch

    // the send op cond command sets the card into the ready state, now we can go into the identification state

    // the send all cid command gives us the cards unique identification number and enters identification state

    if (!send_command(cmd::all_send_cid, 0)) {
        uart::write("EMMC: failed to send all send cid\n");
        return false;
    }

    // copy response over to the cid array
    mem::copy(cid, response, 4 * sizeof(u32));

    uart::write("EMMC: sd card cid: {:08x}{:08x}{:08x}{:08x}\n", cid[0], cid[1], cid[2], cid[3]);

    // now we need to get the active cards relative address
    if (!send_command(cmd::send_relative_addr, 0)) {
        uart::write("EMMC: failed to send relative address\n");
        return false;
    }
    this->rca = response[0] & 0xffff0000;

    uart::write("EMMC: sd card rca: {:08x}\n", rca);

    // getting the rca moves the card into the stand-by state

    // now that we are out of identification we can initiate a higher clock speed
    if (!set_clock_rate(clock_rate_normal)) {
        uart::write("EMMC: failed to set clock rate\n");
        return false;
    }

    // now we know which card we have, now we need to select it
    if (!send_command(cmd::select_card, rca)) {
        uart::write("EMMC: failed to select card\n");
        return false;
    }

    // now we are in transfer mode, but not done. we need to get the sd cards capabilities to know how to transfer
    // data with the card, for this we need to get the SCR register

    // but now the card is in the transfer state, so it doesn't send data via the response register
    // but via the data transfer protocol
    u32 buf[2];

    this->buffer = buf;
    this->blocks_to_transfer = 1;
    this->block_size = 8;

    if (!send_app_command(cmd::send_scr, 0)) {
        uart::write("EMMC: failed to send scr\n");
        return false;
    }

    // we need to bswap the scr, as it's big endian
    buf[0] = bswap32(buf[0]);
    buf[1] = bswap32(buf[1]);

    // swap the bytes
    u32 buf0 = buf[0];
    buf[0] = buf[1];
    buf[1] = buf0;

    this->scr = *(reg::scr*)buf;

    // TODO: switch over to SDHC mode

    // now we can check if we can use 4 bit data transfer or only 1 bit
    if (this->scr.sd_bus_widths & 4) { // 4 bit supported
        // inform sd card of our 4 bit bus width
        if (!send_app_command(cmd::set_bus_width, rca | 2)) {
            uart::write("EMMC: failed to set bus width\n");
            return false;
        }
        // also tell the emmc controller that we can use 4 bit bus
        bus->control0 |= control0::hctl_dwidth;
    }

    // set the block size to the full 512 bytes
    this-> block_size = 512;

    // now we are ready to transfer data

    return true;
}

bool emmc::sd::transfer_block(u32 lba, u32 *buffer, u32 num, bool write) {
    // wait for data inhibit flag
    u32 tout = 0;
    for (; tout < 10000; ++tout) {
        timer::wait_us(10);
        if (!(bus->status & status::dat_inhibit)) {
            break;
        }
    }

    if (tout == 10000) {
        uart::write("EMMC: timeout waiting for data inhibit\n");
        return false;
    }

    if (!sdhc) {
        // sdsc cards require byte addresses
        lba *= 512;
    }

    this->blocks_to_transfer = num;

    reg::cmdtm command;
    if (write) {
        if (num == 1) {
            command = cmd::write_single_block;
        } else {
            command = cmd::write_multiple_block;
        }
    } else {
        if (num == 1) {
            command = cmd::read_single_block;
        } else {
            command = cmd::read_multiple_block;
        }
    }

    this->buffer = buffer;

    for (int i = 0; i < 4; i++) {
        if (send_command(command, lba)) {
            return true;
        }
    }

    uart::write("EMMC: failed to transfer block\n");
    return false;
}

bool emmc::sd::send_app_command(reg::cmdtm command, u32 arg) {
    if (!send_command(cmd::app_cmd, rca)) {
        uart::write("EMMC: failed to send app command\n");
        return false;
    }

    if (rca != 0 && response[0] == 0) {
        uart::write("EMMC: invalid app command response\n");
        return false;
    }

    return send_command(command, arg);
}

u32 emmc::sd::get_base_clock_rate() const {
    mailbox::property_message<mailbox::request_clock_rate> message {
        mailbox::clock_id::emmc
    };

    mailbox::call_property(message);

    auto response = message.get_tag<mailbox::request_clock_rate>();

    if (response.rate == 0 || !(response.code & mailbox::response::success)) {
        return 0;
    }

    return response.rate;
}

static u8 find_lowest_set_bit(u32 x) {
    u8 shift = 32;
    if(!x)
        shift = 0;
    else {
        if (!(x & 0xffff0000u)) { x <<= 16; shift -= 16; }
        if (!(x & 0xff000000u)) { x <<= 8;  shift -= 8; }
        if (!(x & 0xf0000000u)) { x <<= 4;  shift -= 4; }
        if (!(x & 0xc0000000u)) { x <<= 2;  shift -= 2; }
        if (!(x & 0x80000000u)) { x <<= 1;  shift -= 1; }
        if (shift > 0) shift--;
        if (shift > 7) shift = 7;
    }

    return shift;
}

bool emmc::sd::set_clock_rate(u32 rate) const {
    u32 tout = 0;

    // wait for any pending transaction to complete
    for (; tout < 10000; ++tout) {
        timer::wait_us(10);
        if (!(bus->status & ( status::cmd_inhibit | status::dat_inhibit ))) {
            break;
        }
    }

    if (tout == 10000) {
        uart::write("ERROR: timeout waiting for inhibit flag\n");
        return false;
    }

    // disable the device clock
    bus->control1 &= ~control1::clk_en;

    u32 target = base_clock_rate / rate;

    u32 divisor = target;

    // if we are below host spec version 3, we need to give the divisor as a power of 2
    if (host_version <= host_spec::v2) {
        // find the lowest set bit
        u32 shift = find_lowest_set_bit(target - 1);

        uart::write("sd_clk shift {:08x}\n", shift);

        // divisor is now a power of 2 divisor
        divisor = (1 << shift);
    }

    // make sure we are not above the maximum rate
    if (divisor <= 2) {
        divisor = 2;
    }

    uart::write("sd_clk divisor {:08x}\n", divisor);

    divisor = (divisor & 0xff) << 8;

    if (host_version > host_spec::v2) {
        // v3+ supports 10bit divisor
        divisor |= (divisor & 0x300) >> 2;
    }

    // update clock divider bits
    auto cntl = bus->control1;
    // make sure to clear out the old clock divider
    cntl &= ~control1::clk_div_mask;
    cntl |= divisor;

    bus->control1 = cntl;
    timer::wait_us(10);

    bus->control1 |= control1::clk_en;
    timer::wait_us(10);

    tout = 0;
    for (; tout < 10000; ++tout) {
        timer::wait_us(10);
        if (bus->control1 & control1::clk_stable) {
            break;
        }
    }

    if (tout == 10000) {
        uart::write("ERROR: failed to get stable clock\n");
        return false;
    }
    return true;
}

bool emmc::sd::common_io_op(u64 address, u8 *buffer, size_t size, bool write) {
    u32 overflow[512]; // overflow buffer

    u32 first_blocks = size / block_size;
    u32 overflow_size = size % block_size;

    u32 block_address = address / block_size;

    // make first transfer
    if (!transfer_block(block_address, reinterpret_cast<u32*>(buffer), first_blocks, write)) {
        return false;
    }

    if (overflow_size) {
        if (write)
            mem::copy(overflow, buffer + first_blocks, overflow_size);
        // make second transfer
        if (!transfer_block(block_address + first_blocks, overflow, 1, write)) {
            return false;
        }
        // copy overflow to buffer
        if (!write)
            mem::copy(buffer + first_blocks, overflow, overflow_size);
    }

    return true;
}