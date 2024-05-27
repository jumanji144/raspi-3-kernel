#pragma once
#include <common.h>
#include <common/format.h>
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

        enum cmd_type : u8 {
            normal,
            suspend,
            resume,
            abort
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
            cmd_type type : 2{};
            u8 index : 6{};
            unsigned reserved4 : 2{};
        };

        inline constexpr cmdtm invalid_command = {
                .resa = 1, .blkcnt_en = 1, .auto_cmd_en = 3, .dir = data_dir::card_to_host, .multi_blk = 1, .resb = 0xF,
                .rspns_type = rt48busy, .res0 = 1, .crcchk_en = 1, .ixchk_en = 1, .isdata = 1, .type = abort,
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

        struct interrupt {
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

            [[nodiscard]] bool is_error() const volatile {
                return ack_err || cmd_timeout || crc_err || end_bit_err || index_err || data_timeout || data_crc_err ||
                       data_end_bit_err || acmd_err;
            };

            [[nodiscard]] bool is_timeout() const volatile {
                return cmd_timeout || data_timeout;
            };
        };

        struct slotisr_ver {
            u8 slot_status : 8;
            unsigned reserved0 : 8;
            u8 sdversion : 8;
            u8 vendor : 8;
        };

    }

    namespace control0 {
        constexpr u32 hctl_hs_en = 1 << 2;
        constexpr u32 hctl_dwidth = 1 << 1;
    }

    namespace control1 {
        constexpr u32 srst_hc = 1 << 24;
        constexpr u32 srst_cmd = 1 << 25;
        constexpr u32 srst_data = 1 << 26;

        constexpr u32 clk_div_mask = 0x3ff << 6;

        constexpr u32 tounit(u32 x) {
            if(x > 0xF) unreachable();
            return x << 16;
        }

        constexpr u32 clk_en = 1 << 2;
        constexpr u32 clk_stable = 1 << 1;
        constexpr u32 clk_intlen = 1 << 0;
    }

    namespace status {
        constexpr u32 cmd_inhibit = 1 << 0;
        constexpr u32 dat_inhibit = 1 << 1;
        constexpr u32 dat_active = 1 << 2;
        constexpr u32 write_transfer = 1 << 8;
        constexpr u32 read_transfer = 1 << 9;
    }

    namespace interrupt {

        constexpr u32 cmd_done = 1 << 0;
        constexpr u32 write_ready = 1 << 4;
        constexpr u32 read_ready = 1 << 5;

        constexpr u32 error_mask = 0xffff0000;

        inline reg::interrupt get(u32 val) {
            return *reinterpret_cast<reg::interrupt*>(&val);
        }

    }

    namespace slotisr_ver {

        inline reg::slotisr_ver get(u32 val) {
            return *reinterpret_cast<reg::slotisr_ver*>(&val);
        }

    }

    enum host_spec {
        v1 = 0,
        v2 = 1,
        v3 = 2
    };


    struct regs {
        u32 arg2;
        u32 blksizecnt;
        u32 arg1;
        u32 cmdtm;
        u32 resp[4];
        u32 data;
        u32 status;
        u32 control0;
        u32 control1;
        u32 interrupt;
        u32 interrupt_mask;
        u32 interrupt_en;
        u32 control2;
        u32 capabilities1;
        u32 capabilities2;
        u32 reserved0[2]; // 0x48 - 0x50
        u32 force_irpt;
        u32 reserved1[7]; // 0x54 - 0x70
        u32 boot_timeout;
        u32 dbg_sel;
        u32 reserved2[2]; // 0x78 - 0x80
        u32 exrdfifo_cfg;
        u32 exrdfifo_en;
        u32 tune_step;
        u32 tune_steps_std;
        u32 tune_steps_ddr;
        u32 reserved3[0x17]; // 0x94 - 0xF0
        u32 spi_int_spt;
        u32 reserved4[2]; // 0xF4 - 0xFC
        u32 slotisr_ver;
    };

    static volatile regs* bus = (volatile regs*)peripheral::emmc;

    class device {
    public:
        device() = default;
        device(const device&) = delete;
        device& operator=(const device&) = delete;

        virtual bool init();

        [[nodiscard]] const u32& get_block_size() const {
            return this->block_size;
        }

    protected:
        bool wait_for_interrupt(u32 mask, bool clear = true, u32 timeout = 100000);

        [[nodiscard]] virtual u32 get_base_clock_rate() const = 0;
        [[nodiscard]] virtual bool set_clock_rate(u32 rate) const = 0;

        bool send_command(reg::cmdtm command, u32 arg);
        bool reset_command();

        bool do_data_transfer(bool write);

        u32 host_version {};
        u32 base_clock_rate{};

        u32 response[4];

        u32 blocks_to_transfer = 0;
        u32 block_size = 0;

        u32* buffer = nullptr;

        reg::interrupt last_error;
        bool last_timeout = false;
    };

}