#include "kernel/peripheral/sd.h"
#include "kernel/peripheral/timer.h"
#include "kernel/peripheral/gpio.h"
#include "kernel/peripheral/emmc.h"
#include "kernel/peripheral/uart1.h"
#include "kernel/mailbox/mailbox.h"
#include "kernel/devices/sd.h"


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

    emmc::device::init();

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
    byte_access_mode = !sdhc; // 0 means byte access mode
    sdxc = response[0] & ocr::sdxc;
    voltage_1_8 = response[0] & ocr::voltage_1_8;

    uart::write("EMMC: sd card supports SDHC: {}, SDXC: {}, 1.8V: {}\n", sdhc, sdxc, voltage_1_8);

    // if config has 1.8V enabled and we support it, then do a voltage switch
    if (voltage_1_8 && cfg.enable_1_8V) {
        if (!voltage_switch()) {
            // in this case the card refused to switch to 1.8V or is already in 1.8V mode
            // we don't want to abort the card initialization process, so we just call init again
            // but without 1.8V support
            cfg.enable_1_8V = false;

            return this->init();
        }
    }

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

    // parse the csd
    if (!parse_csd()) {
        uart::write("EMMC: failed to parse csd\n");
        return false;
    }

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

    if (this->host_version > 1 && cfg.enable_high_speed) {
        if (!high_speed_switch()) {
            return false;
        }
    }

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
    this->block_size = 512;

    // now we are ready to transfer data

    return true;
}

bool emmc::sd::parse_csd() {
    if (!send_command(cmd::send_csd, rca)) {
        uart::write("EMMC: failed to send csd\n");
        return false;
    }

    mem::copy(&csd, response, 4 * sizeof(u32));

    u8 csd_structure = csd[0] >> 6;

    // get csize field
    u32 blocklen = csd[9] & 0xf;
    u32 csize = 0;
    if (!sdhc) {
        // calculate csize
        csize = ((u32) (csd[8] & 3) << 10) + ((u32) csd[7] << 2) + ((csd[6] & 0xc0) >> 6) + 1;
        u32 mult = ((csd[5] & 0x3) << 1) + ((csd[4] & 0x80) >> 7) + 2;
        csize = (csize << (mult));
    } else {
        csize = ((u32)(csd[7] << 16) + (u32)(csd[6] << 8) + csd[5]) + 1;
        // multiply by KiB
        csize *= (1 << 10);
    }

    this->block_count = csize;
    this->block_size = 1 << blocklen;
    return true;
}

bool emmc::sd::voltage_switch() {
    if (!send_command(cmd::voltage_switch, 0)) {
        uart::write("EMMC: failed to send voltage switch\n");
        return false;
    }

    // now we need to check if the data lines went high

    u8 data = status::dat_level0(bus->status); // get data lines from the status register
    if (data != 0b1111) {
        uart::write("EMMC: data lines did not go high after voltage switch\n");
        return false;
    }

    return true;
}

bool emmc::sd::high_speed_switch() {
    u8 resp[64]; // 512 bit function response register

    this->buffer = reinterpret_cast<u32*>(resp);
    this->block_size = 64;
    this->blocks_to_transfer = 1;

    // inquiry the function register in group one access mode for access modes
    if (!send_command(cmd::switch_func, 0x00FFFFF0)) {
        uart::write("EMMC: failed to send switch func\n");
        return false;
    }

    // check if our card supports high speed
    // function support group 1 contains a bit field with all the supported functions
    // its in the area: 415:400, because the structure is in big endian, byte 14
    // represents the lower byte of the support group one
    u8 support_group1 = resp[13];

    // according to the sd specification bits 0 - 4 select the following speeds:
    // 0: SDR12
    // 1: SDR25
    // 2: SDR50
    // 3: SDR104
    // 4: DDR50

    // where sdr12 is always supported

    bool sdr25 = support_group1 >> 1 & 1;
    bool sdr50 = support_group1 >> 2 & 1;
    bool sdr104 = support_group1 >> 3 & 1;
    bool ddr50 = support_group1 >> 4 & 1;

    uart::write("EMMC: sd card supports SDR25: {}, SDR50: {}, SDR104: {}, DDR50: {}\n", sdr25, sdr50, sdr104, ddr50);

    // select the highest speed that is supported
    // note:: ddr50 only uses 50Mhz but has a transfer rate of 50 MB/s, therefore it is above sdr25, and we favor it
    // over sdr50
    u32 switch_mode = 0;
    u32 clock_rate = 0;
    if (sdr104) {
        switch_mode = 0x80FFFFF3;
        clock_rate = clock_rate_sdr104;
    } else if (ddr50) {
        switch_mode = 0x80FFFFF4;
        clock_rate = clock_rate_ddr50;
    } else if (sdr50) {
        switch_mode = 0x80FFFFF2;
        clock_rate = clock_rate_sdr50;
    } else if (sdr25) {
        switch_mode = 0x80FFFFF1;
        clock_rate = clock_rate_sdr25;
    } else {
        // none are supported, but that's ok
        return true;
    }

    uart::write("EMMC: switching to {}MHz\n", clock_rate / 1000000);

    if (!send_command(cmd::switch_func, switch_mode)) {
        uart::write("EMMC: failed to send switch func\n");
        return false;
    }

    if (!set_clock_rate(clock_rate)) {
        uart::write("EMMC: failed to set clock rate\n");
        return false;
    }

    return true;
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