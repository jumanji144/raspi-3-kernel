#pragma once
#include <kernel/peripheral/emmc.h>

#define SD_OK                0
#define SD_TIMEOUT          (-1)
#define SD_ERROR            (-2)

namespace emmc {

    namespace cmd {
        // normal commands
        constexpr reg::cmdtm go_idle_state = { .index = 0 };
        constexpr reg::cmdtm all_send_cid = { .rspns_type = reg::res_type::rt136, .index = 2 };
        constexpr reg::cmdtm switch_func = { .dir = reg::data_dir::card_to_host, .rspns_type = reg::res_type::rt48,
                                            .isdata = true, .index = 6 };
        constexpr reg::cmdtm send_relative_addr = { .rspns_type = reg::res_type::rt48, .index = 3 };
        constexpr reg::cmdtm select_card = { .rspns_type = reg::res_type::rt48, .index = 7 };
        constexpr reg::cmdtm send_if_cond = { .rspns_type = reg::res_type::rt48, .index = 8 };
        constexpr reg::cmdtm voltage_switch = { .rspns_type = reg::res_type::rt48, .index = 11 };

        // app commands
        constexpr reg::cmdtm app_cmd = { .rspns_type = reg::res_type::rt48, .index = 55 };
        constexpr reg::cmdtm set_bus_width = { .rspns_type = reg::res_type::rt48, .index = 6 };
        constexpr reg::cmdtm send_op_cond = { .rspns_type = reg::res_type::rt48, .index = 41 };
        constexpr reg::cmdtm send_scr = { .dir = reg::data_dir::card_to_host, .rspns_type = reg::res_type::rt48,
                                          .isdata = true, .index = 51 };

        // data transfer commands
        constexpr reg::cmdtm read_single_block = { .dir = reg::data_dir::card_to_host, .rspns_type = reg::res_type::rt48,
                                                   .isdata = true, .index = 17 };
        constexpr reg::cmdtm read_multiple_block = { .dir = reg::data_dir::card_to_host, .multi_blk = true,
                                                     .rspns_type = reg::res_type::rt48, .isdata = true, .index = 18 };

        constexpr reg::cmdtm write_single_block = { .dir = reg::data_dir::host_to_card, .rspns_type = reg::res_type::rt48,
                                                    .isdata = true, .index = 24 };
        constexpr reg::cmdtm write_multiple_block = { .dir = reg::data_dir::host_to_card, .multi_blk = true,
                                                     .rspns_type = reg::res_type::rt48, .isdata = true, .index = 25 };
    }

    namespace ocr {
        constexpr u32 base = 0x00FF8000;
        constexpr u32 sdhc = 1 << 30;
        constexpr u32 sdxc = 1 << 28;
        constexpr u32 voltage_1_8 = 1 << 24;
    }

    namespace reg {
        struct scr {
            unsigned reserved0 : 32;
            u8 cmd_support : 5;
            unsigned reserved1 : 1;
            bool sd_spec_x : 1;
            bool sd_spec_4 : 1;
            u8 ex_security : 4;
            bool sd_spec_3 : 1;
            u8 sd_bus_widths : 4;
            u8 sd_security : 2;
            bool data_stat_after_erase : 1;
            u8 sd_spec : 4;
            u8 scr_structure : 4;
        };
    }

    class sd : public device {
        static constexpr u32 clock_rate_identification = 400000;
        static constexpr u32 clock_rate_normal = 25000000; // 25Mhz, 12.5MB/s
        static constexpr u32 clock_rate_sdr25 = 50000000; // 50Mhz, 25MB/s
        static constexpr u32 clock_rate_sdr50 = 100000000; // 100Mhz, 50MB/s
        static constexpr u32 clock_rate_sdr104 = 208000000; // 208Mhz, 104MB/s
        static constexpr u32 clock_rate_ddr50 = 50000000; // 50Mhz, 50MB/s
    public:

        struct config {
            bool enable_high_speed {};
            bool enable_1_8V {};
        };

        sd(config cfg) : cfg(cfg) {}
        sd() = default;

        bool init() override;

        bool read(u64 address, u8* buffer, size_t size) {
            return common_io_op(address, buffer, size, false);
        }
        bool write(u64 address, const u8* buffer, size_t size) {
            return common_io_op(address, const_cast<u8*>(buffer), size, true);
        }

        bool transfer_block(u32 lba, u32* buffer, u32 num, bool write = false);

    protected:
        bool common_io_op(u64 address, u8* buffer, size_t size, bool write);
        bool send_app_command(reg::cmdtm command, u32 arg);

        bool voltage_switch();
        bool high_speed_switch();

        [[nodiscard]] u32 get_base_clock_rate() const override;
        [[nodiscard]] bool set_clock_rate(u32 rate) const override;

        u32 rca {};
        config cfg {};
        alignas(sizeof(reg::scr)) reg::scr scr {};
        alignas(sizeof(u64)) u32 cid[4]{};
        bool sdhc {};
        bool sdxc {};
        bool voltage_1_8 {};
    };

}

int sd_init();
int sd_readblock(unsigned int lba, unsigned char *buffer, unsigned int num);