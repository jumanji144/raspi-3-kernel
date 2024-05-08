#pragma once
#include <common.h>

template <size_t size>
class Bitset {
public:
    bool get(size_t index) {
        return data[index / 8] & (1 << (index % 8));
    }

    void set(size_t index, bool value) {
        if (value) {
            data[index / 8] |= 1 << (index % 8);
        } else {
            data[index / 8] &= ~(1 << (index % 8));
        }
    }

    size_t find_first() {
        for (size_t i = 0; i < size; i++) {
            if (!get(i)) {
                return i;
            }
        }
        return size;
    }

private:
    u8 data[size / 8 + 1] = {0};
};