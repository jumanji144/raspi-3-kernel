#include <common/memory/mem.h>

void mem::set(void* dest, int c, size_t n) {
    for (size_t i = 0; i < n; i++) {
        ((char*)dest)[i] = c;
    }
}

void mem::copy(void* dest, const void* src, size_t n) {
    for (size_t i = 0; i < n; i++) {
        ((char*)dest)[i] = ((char*)src)[i];
    }
}

// for compiler routines
extern "C" void memcpy(void* dest, const void* src, size_t n) {
    mem::copy(dest, src, n);
}

extern "C" void memset(void* dest, int c, size_t n) {
    mem::set(dest, c, n);
}