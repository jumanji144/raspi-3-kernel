#pragma once
#include "../../common.h"
#include "../bitset.h"

namespace mem {

    template <typename T, size_t n>
    class Slab {
    public:

        void* alloc() {
            size_t index = block_map.find_first();
            block_map.set(index, true);

            return blocks[index];
        }

        void free(void* ptr) {
            auto index = (ssize_t)(ptr - blocks);
            if (index < 0 || index >= n) {
                return;
            }

            block_map.set(index, false);
        }

    private:
        Bitset<n> block_map {};
        T blocks[n];
    };
}