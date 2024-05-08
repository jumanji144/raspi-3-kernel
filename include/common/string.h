#pragma once
#include <common.h>

namespace str {


    size_t strlen(const char* str);
    size_t itos(int64_t n, char* buffer, int base = 10, u8 width = 0);

}