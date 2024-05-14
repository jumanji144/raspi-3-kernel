#pragma once
#include <common.h>
#include <kernel/gpu/font.h>

namespace gpu {
    struct [[gnu::packed]] pixel {
        u8 r;
        u8 g;
        u8 b;
        u8 a;
    };

    struct buffer {
        u32 width;
        u32 height;
        u32 pitch;
        pixel* data;
        u32 buffer_size;

        u32 char_width;
        u32 char_height;
        u32 cursor_x;
        u32 cursor_y;

        pixel& operator()(u32 x, u32 y) {
            return data[y * width + x];
        }

        const pixel& operator()(u32 x, u32 y) const {
            return data[y * width + x];
        }

        [[nodiscard]] pixel& pxl(u32 x, u32 y) const {
            return data[y * width + x];
        }

        void putc(char c);
        void puts(const char* str);

        void clear(pixel color) const {
            for (u32 y = 0; y < height; y++) {
                for (u32 x = 0; x < width; x++) {
                    pxl(x, y) = color;
                }
            }
        }
    };

    buffer allocate_framebuffer(u32 width, u32 height, u32 depth);
}