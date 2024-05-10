#include <common/string.h>

using namespace str;

size_t str::itos(int64_t value, char* buffer, int base, u8 width, bool upper) {
    char buf[64];
    char *ptr = buf;

    uint64_t digit = 0;
    uint64_t v;

    int negative = (base == 10 && value < 0);
    if(negative) {
        v = -value;
    } else {
        v = (uint64_t) value;
    }

    char char_base = upper ? 'A' : 'a';

    while(v || ptr == buf) {
        digit = v % base;
        v /= base;
        if(digit < 10) {
            *ptr++ = digit + '0';
        } else {
            *ptr++ = digit + char_base - 10;
        }
    }

    if (width > 0) {
        // pad with zeros
        while(ptr - buf < width) {
            *ptr++ = '0';
        }
    }

    char* str = buffer;
    size_t size = ptr - buf + negative;

    if (negative) {
        *str++ = '-';
    }

    while(ptr != buf) {
        *str++ = *--ptr;
    }

    *str = '\0';

    return size;
}

u64 str::stou(str::view& view, int base) {
    u64 value = 0;
    for (size_t i = 0; i < view.size; i++) {
        char c = view.data[i];
        if (c >= '0' && c <= '9') {
            value = value * base + (c - '0');
        } else if (c >= 'a' && c <= 'f') {
            value = value * base + (c - 'a' + 10);
        } else if (c >= 'A' && c <= 'F') {
            value = value * base + (c - 'A' + 10);
        } else {
            break;
        }
    }
    return value;
}