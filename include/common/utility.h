#pragma once
#include <common.h>
#include <common/types/type_traits.h>

namespace util {

    template<typename T>
    T&& forward(typename type::remove_reference<T>::type& t) noexcept {
        return static_cast<T&&>(t);
    }

    template<typename T>
    T&& forward(typename type::remove_reference<T>::type&& t) noexcept {
        return static_cast<T&&>(t);
    }

    template<typename T, size_t N>
    struct array {
        T data[N];
        constexpr T& operator[](size_t i) {
            return data[i];
        }

        constexpr const T& operator[](size_t i) const {
            return data[i];
        }

        [[nodiscard]] constexpr size_t size() const {
            return N;
        }

        // iterator support
        constexpr T* begin() {
            return data;
        }

        constexpr T* end() {
            return data + N;
        }

        constexpr array() : data{} {}
    };


}