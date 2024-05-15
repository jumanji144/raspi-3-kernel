#pragma once
#include <common.h>
#include <kernel/peripheral/peripheral.h>

namespace emmc {

    namespace reg {
        struct blksizecnt {
            u16 blksize : 10;
            u16 reserved : 6;
            u16 blkcnt : 16;
        };

        enum res_type : u8 {
            none,
            rt136,
            rt48,
            rt48busy
        };

        enum data_dir : u8 {
            host_to_card,
            card_to_host
        };

        struct cmdtm {
            unsigned resa : 1{};
            bool blkcnt_en : 1{};
            u8 auto_cmd_en : 2{};
            data_dir dir : 1 {};
            bool multi_blk : 1{};
            unsigned resb : 10{};
            res_type rspns_type : 2 {};
            unsigned res0 : 1{};
            bool crcchk_en : 1{};
            bool ixchk_en : 1{};
            bool isdata : 1{};
            u8 type : 2{};
            u8 index : 6{};
            unsigned reserved4 : 2{};
        };

        inline constexpr cmdtm invalid_command = {
                .resa = 1, .blkcnt_en = 1, .auto_cmd_en = 3, .dir = data_dir::card_to_host, .multi_blk = 1, .resb = 0xF,
                .rspns_type = rt48busy, .res0 = 1, .crcchk_en = 1, .ixchk_en = 1, .isdata = 1, .type = 3,
                .index = 0xF, .reserved4 = 1
        };

        struct status {
            bool cmd_inhibit : 1;
            bool dat_inhibit : 1;
            bool dat_active : 1;
            unsigned reserved0 : 5;
            bool write_transfer : 1;
            bool read_transfer : 1;
            unsigned reserved1 : 10;
            u8 dat_level0 : 4;
            u8 cmd_level : 1;
            u8 dat_level1 : 4;
            unsigned reserved2 : 3;
        };

        union control0 {
            struct {
                unsigned res0: 1;
                bool hctl_dwidth: 1;
                bool hctl_hs_en: 1;
                unsigned res1: 2;
                bool hctl_8bit_en: 1;
                unsigned res2: 10;
                bool gap_stop: 1;
                bool gap_restart: 1;
                bool readwait_en: 1;
                bool gap_intr_en: 1;
                bool spi_mode: 1;
                bool boot_en: 1;
                bool alt_boot: 1;
                unsigned res3: 9;
            };
            u32 raw;
        };

        union control1 {
            struct {
                bool clk_intlen: 1;
                bool clk_stable: 1;
                bool clk_en: 1;
                unsigned res0: 2;
                bool clk_gensel: 1;
                u8 clk_freq_ms2: 2;
                u8 clk_freq8: 8;
                u8 data_tounit: 4;
                unsigned res1: 4;
                bool srst_hc: 1;
                bool srst_cmd: 1;
                bool srst_dat: 1;
                unsigned res2: 5;
            };
            u32 raw;
        };

        union interrupt {
            struct {
                bool cmd_done: 1;
                bool data_done: 1;
                bool block_gap: 1;
                unsigned reserved0: 1{};
                bool write_ready: 1;
                bool read_ready: 1;
                unsigned reserved1: 2{};
                bool card_interr: 1;
                unsigned reserved2: 3{};
                bool retune_req: 1;
                bool boot_ack_rcvd: 1;
                bool boot_term: 1;
                bool ack_err: 1;
                bool cmd_timeout: 1;
                bool crc_err: 1;
                bool end_bit_err: 1;
                bool index_err: 1;
                bool data_timeout: 1;
                bool data_crc_err: 1;
                bool data_end_bit_err: 1;
                unsigned reserved3: 1{};
                bool acmd_err: 1;
                unsigned reserved4: 7{};
            };
            u32 raw;

            [[nodiscard]] bool is_error() const volatile {
                return ack_err || cmd_timeout || crc_err || end_bit_err || index_err || data_timeout || data_crc_err ||
                       data_end_bit_err || acmd_err;
            };

            void disable_all() volatile {
                this->raw = 0;
            };

            void enable_all() volatile {
                this->raw = 0xFFFFFFFF;
            }

            void clear_error() volatile {
                ack_err = true; cmd_timeout = true; crc_err = true; end_bit_err = true; index_err = true;
                data_timeout = true; data_crc_err = true; data_end_bit_err = true;acmd_err = true;
            };
        };

        struct control2 {
            bool acnox_err : 1;
            bool acto_err : 1;
            bool acrc_err : 1;
            bool aend_err : 1;
            bool abad_err : 1;
            unsigned reserved0 : 2;
            bool notc12_err : 1;
            unsigned reserved1 : 8;
            u8 uhsmode : 3;
            unsigned reserved2 : 4;
            bool tuneon : 1;
            bool tuned : 1;
            unsigned reserved3 : 6;
        };

        struct slotisr_ver {
            u8 slot_status : 8;
            unsigned reserved0 : 8;
            u8 sdversion : 8;
            u8 vendor : 8;
        };

    }

    struct regs {
        u32 arg2;
        reg::blksizecnt blksizecnt;
        u32 arg1;
        reg::cmdtm cmdtm;
        u32 resp[4];
        u32 data;
        reg::status status;
        reg::control0 control0;
        reg::control1 control1;
        reg::interrupt interrupt;
        reg::interrupt interrupt_mask;
        reg::interrupt interrupt_en;
        reg::control2 control2;
        reg::interrupt force_irpt;
        u32 boot_timeout;
        u32 dbg_sel;
        u32 exrdfifo_cfg;
        u32 exrdfifo_en;
        u32 tune_step;
        u32 tune_steps_std;
        u32 tune_steps_ddr;
        u32 spi_int_spt;
        reg::slotisr_ver slotisr_ver;
    };

    class device {
    public:
        device() = default;
        device(const device&) = delete;
        device& operator=(const device&) = delete;

        virtual bool init();
        virtual bool reset();
        bool reset_command();
        bool send_command(reg::cmdtm, u32 arg);
        bool set_clock(u32 freq);

    protected:
        [[nodiscard]] u32 get_clock_divider(u32 freq) const;
        bool data_transfer(reg::cmdtm command);
        bool transfer_block(bool write, u32* buffer);

        volatile regs* bus = (volatile regs*)peripheral::emmc;

        reg::interrupt error{};
        u32* buffer = nullptr;
        u64 base_clock = 0;
        u32 response[4]{};
        u32 blocks = 0;
        u32 block_size = 0;
        u8 spec{};
        u8 vendor{};
        bool success = false;
    };

}