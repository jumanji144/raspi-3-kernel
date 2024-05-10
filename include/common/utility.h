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


}