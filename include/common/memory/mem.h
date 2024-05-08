#pragma once
#include <common.h>

namespace mem {
    void set(void* dest, int c, size_t n);
    void copy(void* dest, const void* src, size_t n);

    void* alloc(size_t size);
    void free(void* ptr);
}

void* operator new(size_t size);
void operator delete(void* ptr) noexcept;
void operator delete(void* ptr, size_t size) noexcept;