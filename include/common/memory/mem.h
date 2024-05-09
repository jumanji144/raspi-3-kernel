#pragma once
#include <common.h>
#include <common/types/type_traits.h>

namespace mem {
    void set(void* dest, int c, size_t n);
    void copy(void* dest, const void* src, size_t n);

    void* alloc(size_t size);
    void free(void* ptr);

    template<class To, class From>
    inline type::enable_if_t<type::is_trivially_copyable_v<To> && type::is_trivially_copyable_v<From>, To>
            bit_cast(const From& src) noexcept {
        static_assert(type::is_default_constructible_v<To>, "To must be trivially constructible");
        To dst;
        copy(&dst, &src, sizeof(To));
        return dst;
    }

}

void* operator new(size_t size);
void operator delete(void* ptr) noexcept;
void operator delete(void* ptr, size_t size) noexcept;