#include <kernel/peripheral/sd.h>
#include "kernel/peripheral/uart1.h"
#include "kernel/peripheral/timer.h"
#include <kernel/mailbox/mailbox.h>

using namespace emmc;

bool sd::check_v2() {
    // check if the card is version 2
    auto cmd = cmd_send_if_cond;
    u32 arg = 0x1AA;
    if (!send_command(cmd, arg)) {
        // check if the error was a timeout
        if (this->error.cmd_timeout) {
            if (!reset_command())
                return false;

            this->v2 = false;
            return true;
        }

        uart::write("Failed to check if card is version 2\n");
        return false;
    }

    if (response[0] != arg) {
        uart::write("Failed to check if card is version 2\n");
        return false;
    }

    this->v2 = true;
    return true;
}

bool sd::send_app_command(reg::cmdtm cmd, u32 arg) {
    auto app = cmd_app_cmd;
    if (relative_card_address != 0) {
        app.rspns_type = reg::rt48; // we need to get the RCA
    }

    if (!send_command(app, relative_card_address)) {
        uart::write("Failed to send app command\n");
        return false;
    }

    if (response[0] == 0 && relative_card_address != 0) {
        uart::write("Failed to send app command\n");
        return false;
    }

    if (!send_command(cmd, arg)) {
        uart::write("Failed to send app command\n");
        return false;
    }

    return true;
}

bool sd::check_op_cond() {
    constexpr u32 max_op_cond_retry = 10;

    // ACMD41
    u32 arg = 0x40FF8000; // 3.3V
    if (v2) {
        arg |= 1 << 30; // check for high capacity
    }

    u32 timeout = 0;
    for(; timeout < max_op_cond_retry; timeout++) {
        if(!send_app_command(cmd_send_op_cond, arg)) return false;

        if (response[0] & (1 << 31)) { // done
            break;
        }
    }
    if (timeout == max_op_cond_retry || !(response[0] & (1 << 31))) {
        uart::write("Failed to send op cond\n");
        return false;
    }
    this->high_capacity = response[0] & (1 << 30);
    this->ocr = (response[0] >> 8) & 0xFFFF;

    return true;
}

bool sd::read_cid() {
    if(!send_command(cmd_send_cid, 0)) {
        uart::write("Failed to read CID\n");
        return false;
    }
    u8* cid_bytes = (u8*)response;
    for (u32 i = 0; i < 16; i++) {
        cid[i] = cid_bytes[i];
    }
    return true;
}

bool sd::get_scr() {
    if (!high_capacity) { // manually set block size to 512
        if (!send_command(cmd_set_blocklen, 512)) {
            uart::write("Failed to set block length\n");
            return false;
        }
    }

    // read the SCR

    // we first need to use the EMMC to transfer the initial block
    bus->blksizecnt.blksize = 8;
    bus->blksizecnt.blkcnt = 1;

    u32 buf[2];
    this->blocks = 1;
    this->block_size = 8;
    this->buffer = buf;

    if(!send_app_command(cmd_send_scr, 0)) {
        uart::write("Failed to send SCR command\n");
        return false;
    }

    // byteswap the SCR
    u32 temp = bswap32(buf[0]);
    buf[0] = bswap32(buf[1]);
    buf[1] = temp;

    scr = *(reg::scr*)buf;

    // see if we support 4 data lines
    if (scr.sd_bus_widths & (1 << 2)) {
        // set the bus width to 4 bits
        if (!send_app_command(cmd_set_bus_width, relative_card_address | 2)) {
            uart::write("Failed to set bus width\n");
            return false;
        }
        bus->control0.hctl_dwidth = true;
    }

    this->block_size = 512; // set block size to 512

    return true;
}

bool sd::get_base_clock() {
    mailbox::property_message<mailbox::request_clock_rate> message {
            { mailbox::clock_id::emmc }
    };

    mailbox::call_property(message);

    auto response = message.get_tag<mailbox::request_clock_rate>();

    if (response.rate == 0) {
        uart::write("EMMC: sd::get_base_clock: Failed to get base clock\n");
        return false;
    }

    this->base_clock = response.rate;

    return true;
}

bool sd::power_on() {
    mailbox::property_message<mailbox::power_state> message {
            { mailbox::power_did::sd, 3 } // on and wait
    };

    mailbox::call_property(message);

    if (message.code != mailbox::response::success) {
        uart::write("Failed to power on SD card\n");
        return false;
    }

    auto response = message.get_tag<mailbox::power_state>();

    if (response.device != mailbox::power_did::sd) {
        uart::write("Failed to power on SD card\n");
        return false;
    }

    return true;
}

bool sd::power_off() {
    mailbox::property_message<mailbox::power_state> message {
            { mailbox::power_did::sd, 2 } // off and wait
    };

    mailbox::call_property(message);

    if (message.code != mailbox::response::success) {
        uart::write("EMMC: sd::power_off: property mailbox call failed\n");
        return false;
    }

    auto response = message.get_tag<mailbox::power_state>();

    if (response.device != mailbox::power_did::sd) {
        uart::write("EMMC: sd::power_off: device mismatch\n");
        return false;
    }

    return true;
}

static inline u32 max_clock_retry = 100000;

bool sd::set_initial_clock() {
    timer::wait_us(100);

    uart::write("EMMC: sd::set_initial_clock: Base clock rate: {}Hz\n", base_clock);

    u32 clock_rate = clock_rate_iden;

    u32 div = get_clock_divider(clock_rate);

    u32 dem = 1;
    if (div != 0)
        dem = 2 * div;
    u32 actual = base_clock / dem;

    uart::write("EMMC: sd::set_initial_clock: Setting clock to: {}Hz, div: {}, actual: {}Hz\n", clock_rate, div, actual);

    bus->control1.clk_freq8 = div & 0xFF;
    bus->control1.clk_freq_ms2 = (div >> 8) & 0x3;

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
        uart::write("EMMC: sd::set_initial_clock: Clock not stable within timeout\n");
        return false;
    }

    // enable clock
    bus->control1.clk_en = true;

    timer::wait_us(2000);

    return true;
}

bool sd::init() {

    // powercycle the card
    uart::write("EMMC: sd::init: Powering SD card...\n");
    if (!power_off()) return false;
    timer::wait_ms(5);
    if (!power_on()) return false;

    uart::write("EMMC: sd::init: Initializing SD card...\n");
    if (!device::init()) return false;

    if (!get_base_clock()) return false;

    // setup clock for identification mode
    if(!set_initial_clock()) return false;

    uart::write("EMMC: sd::init: Set clock for identification mode\n");

    if (!device::reset()) return false;

    // move to idle mode
    if (!send_command(cmd_go_to_idle, 0)) return false;

    // check if we have a host spec v2 card
    if (!check_v2()) return false;

    // power up the card and figure out information
    if (!check_op_cond()) return false;
    // now our sd card should be in the ready state

    // transition into identification mode
    if (!read_cid()) return false;

    // get the RCA
    if(!send_command(cmd_send_rca, 0))
        return false;

    this->relative_card_address = response[0] & 0xFFFF0000;
    // now we are in stand-by mode

    set_clock(clock_rate_data);

    // select the card to switch into transfer mode
    if(!send_command(cmd_select_card, relative_card_address))
        return false;

    if (!get_scr()) return false;

    return true;
}

bool sd::setup_hc_transfer(u32 blocks) {
    if (blocks > 1 && (scr.cmd_support & (1 << 1))) { // we can use CMD23
        if (!send_command(cmd_set_blockcnt, blocks)) {
            uart::write("Failed to set block length\n");
            return false;
        }
    }

    return true;
}

u32 sd::transfer_data(addr lba, u8 *buffer, u32 blocks, bool write) {
    if (blocks == 0)
        return 0;

    reg::cmdtm cmd {};
    if (write) {
        if (blocks == 1) {
            cmd = cmd_write_single;
        } else {
            cmd = cmd_write_multiple;
        }
    } else {
        if (blocks == 1) {
            cmd = cmd_read_single;
        } else {
            cmd = cmd_read_multiple;
        }
    }

    if(high_capacity) {
        setup_hc_transfer(blocks);
    }

    bus->blksizecnt.blksize = block_size;
    bus->blksizecnt.blkcnt = blocks;

    this->buffer = (u32*)buffer;
    this->blocks = blocks;

    if (!send_command(cmd, lba)) {
        uart::write("Failed to send command\n");
        return 0;
    }

    return blocks;
}