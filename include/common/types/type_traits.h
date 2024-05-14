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

    // remove_reference
    template <typename T>
    struct remove_reference {
        using type = T;
    };

    template <typename T>
    struct remove_reference<T&> {
        using type = T;
    };

    template <typename T>
    struct remove_reference<T&&> {
        using type = T;
    };

    template <typename T>
    using remove_reference_t = typename remove_reference<T>::type;

    template <typename T>
    struct remove_cv {
        using type = T;
    };

    template <typename T>
    struct remove_cv<const T> {
        using type = T;
    };

    template <typename T>
    struct remove_cv<volatile T> {
        using type = T;
    };

    template <typename T>
    struct remove_cv<const volatile T> {
        using type = T;
    };

    template <typename T>
    using remove_cv_t = typename remove_cv<T>::type;

    template<typename>
    struct _is_integral_helper
            : public false_type { };

    template<>
    struct _is_integral_helper<bool>
            : public true_type { };

    template<>
    struct _is_integral_helper<char>
            : public true_type { };

    template<>
    struct _is_integral_helper<signed char>
            : public true_type { };

    template<>
    struct _is_integral_helper<unsigned char>
            : public true_type { };

    // We want is_integral<wchar_t> to be true (and make_signed/unsigned to work)
    // even when libc doesn't provide working <wchar.h> and related functions,
    // so don't check _GLIBCXX_USE_WCHAR_T here.
    template<>
    struct _is_integral_helper<wchar_t>
            : public true_type { };

    template<>
    struct _is_integral_helper<char16_t>
            : public true_type { };

    template<>
    struct _is_integral_helper<char32_t>
            : public true_type { };

    template<>
    struct _is_integral_helper<short>
            : public true_type { };

    template<>
    struct _is_integral_helper<unsigned short>
            : public true_type { };

    template<>
    struct _is_integral_helper<int>
            : public true_type { };

    template<>
    struct _is_integral_helper<unsigned int>
            : public true_type { };

    template<>
    struct _is_integral_helper<long>
            : public true_type { };

    template<>
    struct _is_integral_helper<unsigned long>
            : public true_type { };

    template<>
    struct _is_integral_helper<long long>
            : public true_type { };

    template<>
    struct _is_integral_helper<unsigned long long>
            : public true_type { };

    template<typename T>
    struct is_integral : public _is_integral_helper<T> { };

    template <typename T>
    concept integral = is_integral<T>::value;

    template<typename What, typename... Args>
    struct is_present {
        static constexpr bool value = (is_same_v<What, Args> || ...);
    };

    template<typename What, typename... Args>
    inline constexpr bool is_present_v = is_present<What, Args...>::value;


}