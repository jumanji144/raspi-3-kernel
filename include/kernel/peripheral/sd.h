#pragma once
#include <kernel/peripheral/emmc.h>

namespace emmc {

    // commands
    static constexpr reg::cmdtm cmd_go_to_idle = {};
    static constexpr reg::cmdtm cmd_send_if_cond = { .rspns_type = reg::rt48, .crcchk_en = true, .index = 8 };
    static constexpr reg::cmdtm cmd_send_cid = { .rspns_type = reg::rt136, .crcchk_en = true, .index = 2 };
    static constexpr reg::cmdtm cmd_send_rca = { .rspns_type = reg::rt48, .crcchk_en = true, .index = 3 };
    static constexpr reg::cmdtm cmd_set_bus_width = { .rspns_type = reg::rt48, .crcchk_en = true, .index = 6 };
    static constexpr reg::cmdtm cmd_select_card = { .rspns_type = reg::rt48, .crcchk_en = true, .index = 7 };
    static constexpr reg::cmdtm cmd_set_blocklen = { .rspns_type = reg::rt48, .crcchk_en = true, .index = 16 };
    static constexpr reg::cmdtm cmd_set_blockcnt = { .rspns_type = reg::rt48, .crcchk_en = true, .index = 23 };

    static constexpr reg::cmdtm cmd_read_single = {
            .dir = reg::data_dir::card_to_host, .rspns_type = reg::rt48, .crcchk_en = true, .isdata = true, .index = 17
    };
    static constexpr reg::cmdtm cmd_read_multiple = {
            .dir = reg::data_dir::card_to_host, .rspns_type = reg::rt48, .crcchk_en = true, .isdata = true, .index = 18
    };

    static constexpr reg::cmdtm cmd_write_single = {
            .dir = reg::data_dir::host_to_card, .rspns_type = reg::rt48, .crcchk_en = true, .isdata = true, .index = 24
    };
    static constexpr reg::cmdtm cmd_write_multiple = {
            .dir = reg::data_dir::host_to_card, .rspns_type = reg::rt48, .crcchk_en = true, .isdata = true, .index = 25
    };

    // app commands

    static constexpr reg::cmdtm cmd_app_cmd = { .rspns_type = reg::rt48,  .index = 55 };
    static constexpr reg::cmdtm cmd_send_op_cond = { .rspns_type = reg::rt48, .index = 41 };
    static constexpr reg::cmdtm cmd_send_scr = {
            .dir = reg::data_dir::card_to_host, .rspns_type = reg::rt48, .isdata = true, .index = 51
    };

    namespace reg {
        struct scr {
            unsigned reserved0 : 32;
            // cmd support for CMD20, CMD23, CMD48/49, CMD58/59
            u8 cmd_support : 4;
            unsigned reserved1 : 6;
            bool sd_spec4 : 1;
            u8 ex_security : 4;
            bool sd_spec3 : 1;
            // bus width support 1bit, res, 4bit, res
            u8 sd_bus_widths : 4;
            u8 sd_security : 3;
            bool data_stat_after_erase : 1;
            u8 sd_spec : 4;
            u8 scr_structure : 4;
        };
    }

    class sd : public emmc::device {
        inline static constexpr u32 clock_rate_iden = 400000; // 400kHz
        inline static constexpr u32 clock_rate_data = 25000000; // 25MHz
        inline static constexpr u32 clock_rate_data_hs = 50000000; // 50MHz

        inline static constexpr u32 clock_speed_base = 41666666; // 41.666666MHz
    public:
        sd() : device() {
            this->base_clock = clock_speed_base;
        }

        bool init() override;
        //bool reset() override;

        u32 transfer_data(addr lba, u8* buffer, u32 blocks, bool write);

    private:
        bool send_app_command(reg::cmdtm cmd, u32 arg);
        bool check_v2();
        bool check_op_cond();
        bool read_cid();
        bool get_scr();

        bool setup_hc_transfer(u32 blocks);

        alignas(sizeof(u64)) reg::scr scr{};

        bool v2 = false;
        bool high_capacity = false;

        u16 ocr = 0;
        u8 cid[16]{};
        u32 relative_card_address = 0;
    };

}