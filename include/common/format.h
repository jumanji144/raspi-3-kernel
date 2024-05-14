#pragma once
#include <common.h>
#include <common/string.h>
#include "utility.h"

namespace str {
    namespace fmt {

        template <typename T>
        void format(string& buffer, view& fmt, T arg);

        void common_integer_format(str::string& buffer, str::view &fmt, u64 value);

        template <type::integral T>
        void format(string& buffer, view& fmt, T arg) {
            common_integer_format(buffer, fmt, (u64) arg);
        }

    }

    constexpr void format(string buffer, const char* fmt) {
        size_t itr_size = math::min(buffer.size-1, strlen(fmt));
        for (size_t i = 0; i < itr_size; i++) {
            buffer.data[0] = fmt[i];
            buffer.data++;
            buffer.size--;
        }

        // null terminate
        buffer.data[0] = '\0';
    }

    template <typename... Args>
    constexpr void format(string buffer, const char* fmt, Args&& ... args) {
        format_impl(buffer, fmt, util::forward<Args>(args)...);
    }

    template <typename First, typename... Args>
    constexpr void format_impl(string buffer, const char* fmt, First&& first, Args&&... args) {
        if (buffer.size == 0) {
            return;
        }

        size_t itr_size = math::min(buffer.size-1, strlen(fmt));
        for (size_t i = 0; i < itr_size; i++) {
            char c = fmt[i];
            if (c != '{') {
                buffer.data[0] = c;
                buffer.data++;
                buffer.size--;
                continue;
            }

            char next = fmt[i + 1];
            view fmt_view = empty_view;
            // read format modifier
            if(next == ':') {
                i += 2;
                size_t start = i;
                while (fmt[i++] != '}');
                i--; // correct for the increment
                fmt_view = { fmt + start, i - start };
            } else if (next == '{') {
                // escape
                buffer.data[0] = c;
                buffer.data++;
                buffer.size--;
                continue;
            } else {
                i++; // skip '}'
            }

            fmt::format(buffer, fmt_view, first);

            if constexpr (sizeof...(args) == 0) {
                return format(buffer, fmt + i + 1);
            } else {
                return format(buffer, fmt + i + 1, util::forward<Args>(args)...);
            }
        }
    }
}