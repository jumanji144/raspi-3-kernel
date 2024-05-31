#pragma once
#include <kernel/peripheral/emmc.h>

#define SD_OK                0
#define SD_TIMEOUT          (-1)
#define SD_ERROR            (-2)

namespace emmc {

    namespace cmd {

        // app commands
        constexpr reg::cmdtm set_bus_width = { .rspns_type = reg::res_type::rt48, .index = 6 };
        constexpr reg::cmdtm send_op_cond = { .rspns_type = reg::res_type::rt48, .index = 41 };
        constexpr reg::cmdtm send_scr = { .dir = reg::data_dir::card_to_host, .rspns_type = reg::res_type::rt48,
                                          .isdata = true, .index = 51 };

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

        explicit sd(config cfg) : cfg(cfg) {}
        sd() = default;

        bool init() override;

        bool parse_csd();

        bool voltage_switch();
        bool high_speed_switch();

        [[nodiscard]] u32 get_base_clock_rate() const override;
        [[nodiscard]] bool set_clock_rate(u32 rate) const override;

        alignas(sizeof(reg::scr)) reg::scr scr {};
        config cfg {};
        bool sdhc {};
        bool sdxc {};
        bool voltage_1_8 {};
    };

}

int sd_init();
int sd_readblock(unsigned int lba, unsigned char *buffer, unsigned int num);