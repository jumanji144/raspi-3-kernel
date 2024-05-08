#pragma once
#include <kernel/mailbox/mailbox.h>

namespace board {
    using mailbox::tag, mailbox::command, mailbox::command_type, mailbox::device;

    struct serial_request : tag {

        constexpr serial_request() : tag(command { device::board, command_type::get, 4 }, 8, 0) {}
    };

    struct [[gnu::packed]] serial_response : tag {
        u32 serial2;
        u32 serial1;
    };

    struct board_model_request : tag {

        constexpr board_model_request() : tag(command { device::board, command_type::get, 1 }, 4, 0) {}
    };

    struct [[gnu::packed]] board_model_response : tag {
        u32 model;
    };

    struct board_revision_request : tag {

        constexpr board_revision_request() : tag(command { device::board, command_type::get, 2 }, 4, 0) {}
    };

    struct [[gnu::packed]] board_revision_response : tag {
        u32 revision;
    };

    struct cpu_memory_request : tag {

        constexpr cpu_memory_request() : tag(command { device::board, command_type::get, 5 }, 8, 0) {}
    };

    struct [[gnu::packed]] cpu_memory_response : tag {
        u32 base;
        u32 size;
    };

    struct video_memory_request : tag {

        constexpr video_memory_request() : tag(command { device::board, command_type::get, 6 }, 8, 0) {}
    };

    struct [[gnu::packed]] video_memory_response : tag {
        u32 base;
        u32 size;
    };

    struct firmware_revision_request : tag {

        constexpr firmware_revision_request() : tag(command { device::video_core, command_type::get, 1 }, 4, 0) {}
    };

    struct [[gnu::packed]] firmware_revision_response : tag {
        u32 revision;
    };
}