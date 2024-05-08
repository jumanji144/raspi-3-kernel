#pragma once
#include <kernel/mailbox/mailbox.h>

namespace framebuffer {

    // messages
    struct allocate_buffer_request : mailbox::tag {
        u32 alignment;

        constexpr explicit allocate_buffer_request(u32 alignment) :
            tag({mailbox::device::framebuffer, mailbox::command_type::get, 1}, 4, 0),
            alignment(alignment) {}
    };

    struct allocate_buffer_response : mailbox::tag {
        u32 address;
        u32 size;
    };

    struct release_buffer_request : mailbox::tag {
        constexpr release_buffer_request() :
            tag({mailbox::device::framebuffer, mailbox::command_type::set, 1}, 4, 0) {}
    };

    template <mailbox::command_type type>
    struct physical_dimensions_request : mailbox::tag {
        u32 width;
        u32 height;

        constexpr explicit physical_dimensions_request(u32 width = 0, u32 height = 0) :
            tag({mailbox::device::framebuffer, type, 3}, 8, 0), width(width), height(height) {}
    };

    struct physical_dimensions_response : mailbox::tag {
        u32 width;
        u32 height;
    };

    template <mailbox::command_type type>
    struct virtual_dimensions_request : mailbox::tag {
        u32 width;
        u32 height;

        constexpr explicit virtual_dimensions_request(u32 width = 0, u32 height = 0) :
            tag({mailbox::device::framebuffer, type, 4}, 8, 0), width(width), height(height) {}
    };

    struct virtual_dimensions_response : mailbox::tag {
        u32 width;
        u32 height;
    };

    template <mailbox::command_type type>
    struct depth_request : mailbox::tag {
        u32 depth;

        constexpr explicit depth_request(u32 depth = 0) :
            tag({mailbox::device::framebuffer, type, 5}, 4, 0), depth(depth) {}
    };

    struct depth_response : mailbox::tag {
        u32 depth;
    };

}