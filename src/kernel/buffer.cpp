#include <kernel/mailbox/framebuffer.h>
#include <kernel/gpu/buffer.h>
#include <kernel/gpu/font.h>

using namespace mailbox;

gpu::buffer gpu::allocate_framebuffer(u32 width, u32 height, u32 depth) {
    mailbox::property_message<
            framebuffer::physical_dimensions_request<set>, framebuffer::virtual_dimensions_request<set>,
            framebuffer::virtual_offset_request<set>, framebuffer::depth_request<set>,
            framebuffer::pixel_order_request<set>, framebuffer::allocate_buffer_request,
            framebuffer::pitch_request<get>>
            message {
            {1024, 768}, {1024, 768}, {0, 0}, {32}, {1}, {4096}, {}
    };

    mailbox::call_property(message);

    auto phys_dim = message.get_tag<framebuffer::physical_dimensions_request<set>>();
    auto virt_dim = message.get_tag<framebuffer::virtual_dimensions_request<set>>();
    auto virt_off = message.get_tag<framebuffer::virtual_offset_request<set>>();
    auto depth_res = message.get_tag<framebuffer::depth_request<set>>();
    auto order = message.get_tag<framebuffer::pixel_order_request<set>>();
    auto buffer = message.get_tag<framebuffer::allocate_buffer_request>();
    auto pitch = message.get_tag<framebuffer::pitch_request<get>>();

    return {
        .width = virt_dim.width,
        .height = virt_dim.height,
        .pitch = pitch.pitch,
        .data = reinterpret_cast<gpu::pixel*>(buffer.address),
        .buffer_size = buffer.size,
        .char_width = 8,
        .char_height = 8,
        .cursor_x = 0,
        .cursor_y = 0
    };
}

void gpu::buffer::putc(char c) {
    static constexpr pixel white = {255, 255, 255};
    static constexpr pixel black = {0, 0, 0};

    u32 rows = height / char_height;

    // move cursor
    if (cursor_y >= rows) {
        cursor_y--;
        for (u32 y = 0; y < height - char_height; y++) {
            for (u32 x = 0; x < width; x++) {
                pxl(x, y) = pxl(x, y + char_height);
            }
        }
        for (u32 y = height - char_height; y < height; y++) {
            for (u32 x = 0; x < width; x++) {
                pxl(x, y) = black;
            }
        }
    }

    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        return;
    }

    const u8 *glyph = font8x8_basic[(u8) c];

    for (u32 y = 0; y < char_height; y++) {
        for (u32 x = 0; x < char_width; x++) {
            if (glyph[y] & (1 << x)) {
                pxl(cursor_x * char_width + x, cursor_y * char_height + y) = white;
            } else {
                pxl(cursor_x * char_width + x, cursor_y * char_height + y) = black;
            }
        }
    }

    cursor_x++;
    if (cursor_x >= width / char_width) {
        cursor_x = 0;
        cursor_y++;
    }
}

void gpu::buffer::puts(const char *str) {
    while (*str) {
        putc(*str++);
    }
}