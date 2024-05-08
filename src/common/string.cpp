#include <common/string.h>
#include "common/memory/mem.h"

using namespace str;

size_t str::strlen(const char *str) {
    size_t len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

size_t str::itos(int64_t value, char* buffer, int base, u8 width) {
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

    while(v || ptr == buf) {
        digit = v % base;
        v /= base;
        if(digit < 10) {
            *ptr++ = digit + '0';
        } else {
            *ptr++ = digit + 'a' - 10;
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