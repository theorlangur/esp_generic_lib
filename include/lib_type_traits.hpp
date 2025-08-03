#ifndef LIB_TYPE_TRAITS_HPP_
#define LIB_TYPE_TRAITS_HPP_

#include <inttypes.h>
#include <cstring>
#include <type_traits>
#if __has_include(<expected>)
#include <expected>
#endif

template<class T>
using deref_t = std::remove_cvref_t<decltype(*std::declval<T>())>;

template<class T>
concept simple_destructible_t = std::is_trivially_destructible_v<T>;// || requires { typename T::can_relocate; };

#if __has_include(<expected>)
template<class C>
struct is_expected_type
{
    static constexpr const bool value = false;
};

template<class V, class E>
struct is_expected_type<std::expected<V,E>>
{
    static constexpr const bool value = true;
};

template<class C>
constexpr bool is_expected_type_v = is_expected_type<std::remove_cvref_t<C>>::value;
#endif

template<size_t N>
struct MinSizeType { using type = size_t; };
template<size_t N> requires (N <= 255)
struct MinSizeType<N> { using type = uint8_t; };
template<size_t N> requires (N > 255 && N <= 65535)
struct MinSizeType<N> { using type = uint16_t; };

template<size_t N>
struct MinBitSizeType { using type = size_t; };
template<size_t N> requires (N <= 7)
struct MinBitSizeType<N> { using type = uint8_t; };
template<size_t N> requires (N > 7 && N <= 15)
struct MinBitSizeType<N> { using type = uint16_t; };

#endif
