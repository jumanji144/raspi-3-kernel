#pragma once
#include <stdint.h>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;

using addr = u64;

using size_t = u64;
using ssize_t = s64;

using v32 = volatile u32;

// align sizes
constexpr u32 align_up(u32 value, u32 alignment) {
    return (value + alignment - 1) & ~(alignment - 1);
}

constexpr u32 align_down(u32 value, u32 alignment) {
    return value & ~(alignment - 1);
}

constexpr u32 bswap32(u32 value) {
    return __builtin_bswap32(value);
}

#define offsetof(type, member) __builtin_offsetof(type, member)
