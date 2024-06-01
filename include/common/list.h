#pragma once
#include <common.h>

#include <common/utility.h>

namespace util {

    template<typename T, size_t N>
    struct list {
        T data[N];
        size_t count = 0;

        constexpr T& operator[](size_t i) {
            return data[i];
        }

        constexpr const T& operator[](size_t i) const {
            return data[i];
        }

        [[nodiscard]] constexpr size_t size() const {
            return count;
        }

        // iterator support
        constexpr T* begin() {
            return data;
        }

        constexpr T* end() {
            return data + count;
        }

        constexpr list() : data{} {}

        void add(const T& value) {
            data[count++] = value;
        }

        void add(T&& value) {
            data[count++] = util::forward<T>(value);
        }

        void remove_list() {
            count--;
        }

        void clear() {
            count = 0;
        }

        [[nodiscard]] bool empty() const {
            return count == 0;
        }

        [[nodiscard]] T& last() {
            return data[count - 1];
        }

        [[nodiscard]] const T& last() const {
            return data[count - 1];
        }

        [[nodiscard]] T& first() {
            return data[0];
        }

        [[nodiscard]] const T& first() const {
            return data[0];
        }

        void erase(size_t index) {
            for (size_t i = index; i < count - 1; i++) {
                data[i] = data[i + 1];
            }
            count--;
        }
    };

}