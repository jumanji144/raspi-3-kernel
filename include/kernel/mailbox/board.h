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

}