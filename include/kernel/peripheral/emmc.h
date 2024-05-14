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

        static_assert(sizeof(blksizecnt) == 4);

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

        static_assert(sizeof(cmdtm) == 4);

        inline constexpr cmdtm invalid_command = {
                .resa = 1, .blkcnt_en = 1, .auto_cmd_en = 3, .dir = data_dir::card_to_host, .multi_blk = 1, .resb = 0xF,
                .rspns_type = rt48busy, .res0 = 1, .crcchk_en = 1, .ixchk_en = 1, .isdata = 1, .type = 3,
                .index = 0xF, .reserved4 = 1
        };

        struct status {
            bool cmd_inhibit : 1;
            bool dat_inhibit : 1;
            bool dat_active : 1;
            unsigned reserved0 : 4;
            bool write_transfer : 1;
            bool read_transfer : 1;
            unsigned reserved1 : 10;
            u8 dat_level0 : 3;
            u8 cmd_level : 1;
            u8 dat_level1 : 3;
            unsigned reserved2 : 3;
        };

        struct control0 {
            unsigned res0 : 1;
            bool hctl_dwidth : 1;
            bool hctl_hs_en : 1;
            unsigned res1 : 2;
            bool hctl_8bit_en : 1;
            unsigned res2 : 10;
            bool gap_stop : 1;
            bool gap_restart : 1;
            bool readwait_en : 1;
            bool gap_intr_en : 1;
            bool spi_mode : 1;
            bool boot_en : 1;
            bool alt_boot : 1;
            unsigned res3 : 9;
        };

        struct control1 {
            bool clk_intlen : 1;
            bool clk_stable : 1;
            bool clk_en : 1;
            unsigned res0 : 2;
            bool clk_gensel : 1;
            u8 clk_freq_ms2 : 2;
            u8 clk_freq8 : 8;
            u8 data_tounit : 4;
            unsigned res1 : 4;
            bool srst_hc : 1;
            bool srst_cmd : 1;
            bool srst_dat : 1;
            unsigned res2 : 5;
        };

        struct interrupt {
            bool cmd_done : 1;
            bool data_done : 1;
            bool block_gap : 1;
            unsigned reserved0 : 1{};
            bool write_ready : 1;
            bool read_ready : 1;
            unsigned reserved1 : 2{};
            bool card_interr : 1;
            unsigned reserved2 : 3{};
            bool retune_req : 1;
            bool boot_ack_rcvd : 1;
            bool boot_term : 1;
            bool ack_err : 1;
            bool cmd_timeout : 1;
            bool crc_err : 1;
            bool end_bit_err : 1;
            bool index_err : 1;
            bool data_timeout : 1;
            bool data_crc_err : 1;
            bool data_end_bit_err : 1;
            unsigned reserved3 : 1{};
            bool acmd_err : 1;
            unsigned reserved4 : 7{};

            [[nodiscard]] bool is_error() const volatile {
                return ack_err || cmd_timeout || crc_err || end_bit_err || index_err || data_timeout || data_crc_err ||
                       data_end_bit_err || acmd_err;
            };

            void disable_all() volatile {
                this->cmd_done = false; this->data_done = false; this->block_gap = false; this->write_ready = false;
                this->read_ready = false; this->card_interr = false; this->retune_req = false; this->boot_ack_rcvd = false;
                this->boot_term = false; this->ack_err = false; this->cmd_timeout = false; this->crc_err = false;
                this->end_bit_err = false; this->index_err = false; this->data_timeout = false; this->data_crc_err = false;
                this->data_end_bit_err = false; this->acmd_err = false;
            };

            void enable_all() volatile {
                this->cmd_done = true; this->data_done = true; this->block_gap = true; this->write_ready = true;
                this->read_ready = true; this->card_interr = true; this->retune_req = true; this->boot_ack_rcvd = true;
                this->boot_term = true; this->ack_err = true; this->cmd_timeout = true; this->crc_err = true;
                this->end_bit_err = true; this->index_err = true; this->data_timeout = true; this->data_crc_err = true;
                this->data_end_bit_err = true; this->acmd_err = true;
            }

            void clear_error() volatile {
                this->ack_err = true; this->cmd_timeout = true; this->crc_err = true; this->end_bit_err = true;
                this->index_err = true; this->data_timeout = true; this->data_crc_err = true; this->data_end_bit_err = true;
                this->acmd_err = true;
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