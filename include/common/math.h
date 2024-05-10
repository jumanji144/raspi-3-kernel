#pragma once
#include <common.h>

namespace math {

    constexpr inline u64 max(u64 a, u64 b) {
        return a > b ? a : b;
    }

    constexpr inline u64 min(u64 a, u64 b) {
        return a < b ? a : b;
    }

}