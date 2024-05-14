#pragma once
#include <kernel/mailbox/mailbox.h>

namespace framebuffer {

    struct allocate_buffer_request : mailbox::tag {
        u32 address;
        u32 size;

        constexpr allocate_buffer_request()
            : tag({mailbox::device::framebuffer, mailbox::command_type::get, 1}, 8, 0) {}

        constexpr allocate_buffer_request(u32 alignment)
            : tag({mailbox::device::framebuffer, mailbox::command_type::get, 1}, 8, 0),
                    address(alignment), size(0) {}
    };

    template<mailbox::command_type type = mailbox::command_type::get>
    struct physical_dimensions_request : mailbox::tag {
        u32 width = 0;
        u32 height = 0;

        constexpr physical_dimensions_request()
            : tag({mailbox::device::framebuffer, type, 3}, 8, 0) {}

        constexpr physical_dimensions_request(u32 width, u32 height)
            : tag({mailbox::device::framebuffer, type, 3}, 8, 0), width(width), height(height) {}
    };

    template<mailbox::command_type type = mailbox::command_type::get>
    struct virtual_dimensions_request : mailbox::tag {
        u32 width = 0;
        u32 height = 0;

        constexpr virtual_dimensions_request()
            : tag({mailbox::device::framebuffer, type, 4}, 8, 0) {}

        constexpr virtual_dimensions_request(u32 width, u32 height)
            : tag({mailbox::device::framebuffer, type, 4}, 8, 0), width(width), height(height) {}
    };

    template<mailbox::command_type type = mailbox::command_type::get>
    struct depth_request : mailbox::tag {
        u32 depth = 0;

        constexpr depth_request()
            : tag({mailbox::device::framebuffer, type, 5}, 4, 0) {}

        constexpr depth_request(u32 depth)
            : tag({mailbox::device::framebuffer, type, 5}, 4, 0), depth(depth) {}
    };

    template<mailbox::command_type type = mailbox::command_type::get>
    struct pixel_order_request : mailbox::tag {
        u32 order = 0;

        constexpr pixel_order_request()
            : tag({mailbox::device::framebuffer, type, 6}, 4, 0) {}

        constexpr pixel_order_request(u32 order)
            : tag({mailbox::device::framebuffer, type, 6}, 4, 0), order(order) {}
    };

    template<mailbox::command_type type = mailbox::command_type::get>
    struct pitch_request : mailbox::tag {
        u32 pitch = 0;

        constexpr pitch_request()
            : tag({mailbox::device::framebuffer, type, 8}, 4, 0) {}

        constexpr pitch_request(u32 pitch)
            : tag({mailbox::device::framebuffer, type, 8}, 4, 0), pitch(pitch) {}
    };

    template<mailbox::command_type type = mailbox::command_type::get>
    struct virtual_offset_request : mailbox::tag {
        u32 x = 0;
        u32 y = 0;

        constexpr virtual_offset_request()
            : tag({mailbox::device::framebuffer, type, 9}, 8, 0) {}

        constexpr virtual_offset_request(u32 x, u32 y)
            : tag({mailbox::device::framebuffer, type, 9}, 8, 0), x(x), y(y) {}
    };

}