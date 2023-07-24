#pragma once

#include <utility>

namespace chx::log {
namespace detail {
template <std::size_t Base, std::size_t Exp, std::size_t Value = 1>
constexpr std::size_t pow() noexcept(true) {
    if constexpr (Exp != 0) {
        return pow<Base, Exp - 1, Value * Base>();
    } else {
        return Value;
    }
}
}  // namespace detail

template <char c> using carrier = std::integral_constant<char, c>;

template <char... Cs> struct string {
    template <std::size_t Idx> constexpr auto get() const {
        return get_impl<Idx, Cs...>();
    }
    template <char... OtherCs>
    constexpr string<Cs..., OtherCs...> append() const {
        return {};
    }
    template <char Target> constexpr bool contains() noexcept(true) {
        return ((Target == Cs) || ...);
    }
    constexpr static inline std::size_t size = sizeof...(Cs);
    constexpr static inline decltype(std::make_integer_sequence<
                                     std::size_t, size>{}) iterate = {};
    constexpr static inline char data[sizeof...(Cs) + 1] = {Cs..., 0};
    template <std::size_t Begin> constexpr auto sub() {
        return sub2_impl<0>(std::make_integer_sequence<std::size_t, Begin>());
    }
    template <std::size_t Begin, std::size_t Size> constexpr auto sub() const {
        return sub2_impl<Begin>(
            std::make_integer_sequence<std::size_t, Size>());
    }
    constexpr auto to_lower() const {
        constexpr auto f = [](char x) -> char {
            if (x < 'a') {
                return x + ('a' - 'A');
            } else {
                return x;
            }
        };
        return string<f(Cs)...>();
    }
    constexpr std::size_t to_number() const noexcept(true) {
        return to_number_impl(
            std::make_integer_sequence<std::size_t, sizeof...(Cs)>{});
    }

    template <std::size_t N, typename = std::enable_if_t<N >= sizeof...(Cs)>>
    constexpr void copy_to(char (&p)[N]) const noexcept(true) {
        copy_impl(p, iterate);
    }

  private:
    template <std::size_t Idx, char _C, char... _Cs>
    constexpr auto get_impl() const {
        if constexpr (Idx == 0) {
            return carrier<_C>{};
        } else {
            return get_impl<Idx - 1, _Cs...>();
        }
    }

    template <std::size_t Begin, std::size_t... Is>
    constexpr auto sub2_impl(std::integer_sequence<std::size_t, Is...>) const {
        std::tuple<string<Cs>...> tp;
        return string<>()
            .append<std::tuple_element_t<Is + Begin, decltype(tp)>()
                        .template get<0>()...>();
    }

    template <std::size_t... Is>
    constexpr std::size_t
    to_number_impl(std::integer_sequence<std::size_t, Is...>) const {
        constexpr auto f = [](char c) { return c >= '0' && c <= '9'; };
        static_assert((f(Cs) && ...));
        return (((get<size - Is - 1>() - '0') * detail::pow<10, Is>()) + ...);
    }
    template <std::size_t N, std::size_t... Is>
    constexpr void copy_impl(char (&p)[N],
                             std::integer_sequence<std::size_t, Is...>) const
        noexcept(true) {
        ((p[Is] = Cs), ...);
    }
};

template <char... CsA, char... CsB>
constexpr bool operator==(string<CsA...>, string<CsB...>) noexcept(true) {
    return std::is_same_v<string<CsA...>, string<CsB...>>;
}
template <char... CsA, char... CsB>
constexpr bool operator!=(string<CsA...>, string<CsB...>) noexcept(true) {
    return !std::is_same_v<string<CsA...>, string<CsB...>>;
}
template <char... CsA, char... CsB>
constexpr string<CsA..., CsB...> concat(string<CsA...>,
                                        string<CsB...>) noexcept(true) {
    return {};
}
}  // namespace chx::log
