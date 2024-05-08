#include <kernel/mailbox/framebuffer.h>
#include <kernel/gpu/buffer.h>
#include <kernel/gpu/font.h>

using namespace mailbox;

gpu::buffer gpu::get_framebuffer(u32 width, u32 height, u32 depth) {
    u8 bytes_per_pixel = depth / 8;

    property_message<framebuffer::physical_dimensions_request<command_type::set>> physical_dimensions(width, height);
    property_message<framebuffer::virtual_dimensions_request<command_type::set>> virtual_dimensions(width, height);
    property_message<framebuffer::depth_request<command_type::set>> depth_request(depth);
    property_message<framebuffer::allocate_buffer_request> allocate_framebuffer(16);

    framebuffer::physical_dimensions_response physical_dimension= call_property<>(physical_dimensions)
            .get_property<framebuffer::physical_dimensions_response>().get_tag();
    call_property<>(virtual_dimensions);
    call_property<>(depth_request);
    
    framebuffer::allocate_buffer_response allocate_buffer_response = call_property<>(allocate_framebuffer)
            .get_property<framebuffer::allocate_buffer_response>().get_tag();

    return {physical_dimension.width, physical_dimension.height, physical_dimension.width * bytes_per_pixel,
            (pixel*)(uintptr_t) allocate_buffer_response.address, allocate_buffer_response.size,
            physical_dimension.width / 8, physical_dimension.height / 8, 0, 0 };
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