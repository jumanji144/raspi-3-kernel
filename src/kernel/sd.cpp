#include <kernel/peripheral/sd.h>
#include "kernel/peripheral/uart1.h"

using namespace emmc;

bool sd::check_v2() {
    // check if the card is version 2
    auto cmd = cmd_send_if_cond;
    u32 arg = 0x1AA;
    if (!send_command(cmd, arg)) {
        // check if the error was a timeout
        if (this->error.cmd_timeout)
            return false;

        uart::write("Failed to check if card is version 2\n");
        return false;
    }

    if (response[0] != arg) {
        uart::write("Failed to check if card is version 2\n");
        return false;
    }

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

bool sd::init() {
    uart::write("Initializing SD card...\n");
    if (!device::init()) return false;

    // setup clock for identification mode
    if(!set_clock(clock_rate_iden)) return false;
    if (!device::reset()) return false;

    // move to idle mode
    if (!send_command(cmd_go_to_idle, 0)) return false;

    // check if we have a host spec v2 card
    this->v2 = check_v2();

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