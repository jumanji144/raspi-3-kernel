#pragma once
#include <common.h>
#include <common/math.h>
#include <common/types/type_traits.h>

namespace str {

    constexpr size_t strlen(const char* str) {
        size_t len = 0;
        while (str[len] != '\0') {
            len++;
        }
        return len;
    }

    struct view {
        const char* data;
        size_t size;

        explicit operator bool() const {
            return size != 0;
        }

        explicit operator const char*() const {
            return data;
        }

        constexpr view(const char* data, size_t size) : data(data), size(size) {}
        constexpr view(const char* data) : data(data), size(str::strlen(data)) {}
    };

    inline constexpr view empty_view = { "", 0 };

    struct string {
        char* data;
        size_t size;

        constexpr string(const char* data, size_t size) : data(const_cast<char*>(data)), size(size) {}
        constexpr string(const char* data) : data(const_cast<char*>(data)), size(strlen(data)) {}
    };

    inline constexpr string empty_string = { "", 0 };

    size_t itos(int64_t n, char* buffer, int base = 10, u8 width = 0, bool upper = false);
    u64 stou(view& view, int base = 10);

}