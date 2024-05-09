#pragma once
#include <common.h>

namespace type {

    template<class T, T v>
    struct integral_constant {
        static constexpr T value = v;
        using value_type = T;
        using type = integral_constant;
        constexpr explicit operator value_type() const noexcept { return value; }
        constexpr value_type operator()() const noexcept { return value; }
    };

    using true_type = integral_constant<bool, true>;
    using false_type = integral_constant<bool, false>;

    template<typename T>
    struct is_union
            : public integral_constant<bool, __is_union(T)>
    { };

    namespace details {
        template<typename B>
        true_type test_ptr_conv(const volatile B*);

        template<typename>
        false_type test_ptr_conv(const volatile void*);

        template<typename T, typename D>
        auto test_is_base_of(int) -> decltype(test_ptr_conv<T>(static_cast<D*>(nullptr)));

        template<typename, typename>
        auto test_if_base_of(...) -> true_type;

        template<class T>
        integral_constant<bool, !is_union<T>::value> test_if_class(int T::*);

        template<class>
        false_type test_if_class(...);
    }

    template<class T>
    struct is_class : decltype(details::test_if_class<T>(nullptr)) {};

    template<typename Base, typename Derived>
    struct is_base_of : integral_constant<bool,
                is_class<Base>::value && is_class<Derived>::value &&
                decltype(details::test_is_base_of<Base, Derived>(0))::value> {};

    template<typename Base, typename Derived>
    inline constexpr bool is_base_of_v = is_base_of<Base, Derived>::value;

    template<class T, class U>
    struct is_same : false_type {};

    template<class T>
    struct is_same<T, T> : true_type {};

    template<class T, class U>
    inline constexpr bool is_same_v = is_same<T, U>::value;

    template<typename T>
    struct is_reference : false_type {};

    template<typename T>
    struct is_reference<T&> : true_type {};

    template<class T>
    struct alignment_of : integral_constant<size_t, alignof(T)> {};

    template<typename T>
    inline constexpr size_t alignment_of_v = alignment_of<T>::value;

    template<bool B, class T = void>
    struct enable_if {};

    template<class T>
    struct enable_if<true, T> { typedef T type; };

    template< bool B, class T = void >
    using enable_if_t = typename enable_if<B,T>::type;

    template<typename T>
    struct is_trivially_copyable : integral_constant<bool, __is_trivially_copyable(T)> {};

    template< class T >
    inline constexpr bool is_trivially_copyable_v = is_trivially_copyable<T>::value;

    template<typename T>
    struct is_trivially_constructible : integral_constant<bool, __is_trivially_constructible(T)> {};

    template<typename T>
    inline constexpr bool is_trivially_constructible_v = is_trivially_constructible<T>::value;

    template <typename T, typename... Args>
    inline constexpr bool is_constructible_v = __is_constructible(T, Args...);

    template <typename T>
    inline constexpr bool is_default_constructible_v = __is_constructible(T);


}