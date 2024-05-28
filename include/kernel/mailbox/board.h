#pragma once
#include <kernel/mailbox/mailbox.h>

namespace board {
    using mailbox::tag, mailbox::command, mailbox::command_type, mailbox::device;

    struct serial_request : tag {
        u32 serial2 = 0;
        u32 serial1 = 0;

        constexpr serial_request() : tag(command { device::board, command_type::get, 4 }, 8, 0) {}
    };


    struct board_model_request : tag {
        u32 model = 0;

        constexpr board_model_request() : tag(command { device::board, command_type::get, 1 }, 4, 0) {}
    };


    struct board_revision_request : tag {
        u32 revision = 0;

        constexpr board_revision_request() : tag(command { device::board, command_type::get, 2 }, 4, 0) {}
    };


    struct cpu_memory_request : tag {
        u32 base = 0;
        u32 size = 0;

        constexpr cpu_memory_request() : tag(command { device::board, command_type::get, 5 }, 8, 0) {}
    };


    struct video_memory_request : tag {
        u32 base = 0;
        u32 size = 0;

        constexpr video_memory_request() : tag(command { device::board, command_type::get, 6 }, 8, 0) {}
    };

    struct [[gnu::packed, gnu::aligned(sizeof(u32))]] firmware_revision_request : tag {
        u32 revision = 0;

        constexpr firmware_revision_request() : tag(command { device::video_core, command_type::get, 1 }, 4, 0) {}
    };

    constexpr u32 max_string_size = 1024;
    struct gencmd_request : tag {
        u32 error = 0;
        char cmd[max_string_size] = {0};

        gencmd_request(const char* cmd)
            : tag(command { device::clock, command_type::get, 0x80 }, max_string_size, 0) {
            mem::copy(this->cmd, cmd, str::strlen(cmd) + 1);
        }

        constexpr gencmd_request()
            : tag(command { device::clock, command_type::get, 0x80 }, max_string_size, 0) {}
    };

    /**
     * Warning to any programmers using this, this is to alter the OTP (One time programmable) memory region
     * These Write actions are irreversible, meaning they burn into the silicon and will forever stay this way
     * (Unless you atomically alter the silicon)
     * @tparam num number of u32 values to write
     */
    template <u8 num>
    struct otp_program_region : tag {
        u32 start = 0;
        u32 number = num;
        u32 data[num] = { 0 };

        constexpr otp_program_region(u32 start, const util::array<u32, num>& data)
            : tag(command { device::clock, command_type::set, 0x21 }, 8 + num * 4, 8 + num * 4), start(start) {
            mem::copy(this->data, data.data(), num * 4);
        }

        constexpr otp_program_region()
            : tag(command { device::clock, command_type::set, 0x21 }, 8 + num * 4, 8 + num * 4) {}
    };

    /**
     * Warning to any programmers using, this is to alter the OTP (One time programmable) memory region
     * This action will PERMANENTLY lock the region, meaning it can never be altered again
     */
    struct otp_lock_region : tag {
        u32 start = 0xffffffff;
        u32 lock_cmd = 0xaffe0000;

        constexpr otp_lock_region()
            : tag(command {device::clock, command_type::set, 0x21}, 8, 8) {}
    };

    template <u8 num>
    struct otp_read_region : tag {
        u32 start = 0;
        u32 number = num;
        u32 data[num] = {0};

        constexpr otp_read_region(u32 start)
            : tag(command { device::clock, command_type::get, 0x21 }, 8 + num * 4, 8 + num * 4), start(start) {}

        constexpr otp_read_region()
            : tag(command { device::clock, command_type::get, 0x21 }, 8 + num * 4, 8 + num * 4) {}
    };

}