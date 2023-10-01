#pragma once

#include "./format.hpp"

#include <charconv>
#include <limits>

namespace chx::log {
namespace detail {
template <typename Integer, typename Flag> struct integer_general {};
template <char C> struct integer_base {
    static constexpr std::size_t n1() {
        if constexpr (C == 'x') {
            return 16;
        } else if constexpr (C == 'o') {
            return 8;
        }
    }
    static constexpr std::size_t n2() {
        if constexpr (C == 'x') {
            return 4;
        } else if constexpr (C == 'o') {
            return 3;
        }
    }
};

template <typename Integer, char... SubFlags, char... SubWidth,
          char... SubPrecision>
struct integer_general<
    Integer, flags<sub_flags<SubFlags...>, sub_width<SubWidth...>,
                   sub_precision<SubPrecision...>, sub_additional<'b'>>> {
    using sub_flags_type = sub_flags<SubFlags...>;
    using sub_width_type = sub_width<SubWidth...>;

    template <Integer N, std::size_t R> constexpr static std::size_t log() {
        if constexpr (N != 0) {
            return log<N / 2, R + 1>();
        } else {
            return R;
        }
    }

    constexpr static std::size_t max_digits_impl() {
        if constexpr (std::is_signed_v<Integer> ||
                      sub_flags_type::forced_sign) {
            return log<std::numeric_limits<Integer>::min(), 0>() + 1;
        } else {
            return log<std::numeric_limits<Integer>::max(), 0>();
        }
    }

    constexpr static inline std::size_t max_digits = max_digits_impl();

    constexpr static std::size_t max_size() noexcept(true) {
        if constexpr (!sub_width_type::ast) {
            if constexpr (sub_width_type::norm) {
                if constexpr (sub_width_type().to_number() > max_digits) {
                    return sub_width_type().to_number();
                } else {
                    return max_digits;
                }
            } else {
                return max_digits;
            }
        } else {
            return 0;
        }
    }

    constexpr static std::size_t fast_digit_count(Integer i) noexcept(true) {
        if constexpr (std::is_signed_v<Integer>) {
            if (i != std::numeric_limits<Integer>::min()) {
                if (i < 0) {
                    return fast_digit_count(-i) + 1;
                }
            } else {
                return max_digits;
            }
        }
        return 64 - __builtin_clzll(i | 1);
    }

    constexpr static auto calculate_size(Integer i) noexcept(true) {
        if constexpr (sub_width_type::norm && !sub_flags_type::left_justify) {
            struct result_type {
                std::size_t full_size;
                std::size_t actual_size;
            } res = {};
            res.actual_size = fast_digit_count(i);
            if constexpr (sub_flags_type::forced_sign) {
                if (i >= 0) {
                    ++res.actual_size;
                }
            }
            res.full_size = res.actual_size >= sub_width_type().to_number()
                                ? res.actual_size
                                : sub_width_type().to_number();
            return res;
        } else {
            std::size_t res = fast_digit_count(i);
            if constexpr (sub_flags_type::forced_sign) {
                if (i >= 0) {
                    ++res;
                }
            }
            if constexpr (sub_flags_type::left_justify &&
                          sub_width_type::norm) {
                return std::max(res, sub_width_type().to_number());
            } else {
                return res;
            }
        }
    }

    template <typename RandomAccessIterator, typename SizeType>
    constexpr static std::size_t format(RandomAccessIterator ptr, Integer x,
                                        const SizeType& res) noexcept(true) {
        if constexpr (sub_width_type::norm && !sub_flags_type::left_justify) {
            // the only right-justify branch
            if (res.actual_size != res.full_size) {
                for (std::size_t offset = res.full_size - res.actual_size;
                     offset != 0; --offset) {
                    if constexpr (sub_flags_type::zero_padding) {
                        *(ptr++) = '0';
                    } else {
                        *(ptr++) = ' ';
                    }
                }
            }
            final_format(x, ptr, res.actual_size);
            return res.full_size;
        }  // if(ast and not left-justify) {}
        else {
            final_format(x, ptr, res);
            return res;
        }
    }
    template <typename RandomAccessIterator>
    static void final_format(Integer x, RandomAccessIterator ptr,
                             std::size_t len) noexcept(true) {
        if constexpr (sub_flags_type::forced_sign) {
            if (x >= 0) {
                *(ptr++) = '+';
                --len;
            }
        }
        std::to_chars_result r = std::to_chars(
            detail::iter_addr(ptr), detail::iter_addr(ptr + len), x, 2);
        if constexpr (sub_flags_type::left_justify) {
            const std::size_t num_size = r.ptr - detail::iter_addr(ptr);
            for (len -= num_size, ptr += num_size; len != 0; --len) {
                *(ptr++) = ' ';
            }
        }
    }
};

// temporary implement, need improvement
template <typename Integer, char... SubFlags, char... SubWidth,
          char... SubPrecision, char C>
struct integer_general<Integer,
                       flags<sub_flags<SubFlags...>, sub_width<SubWidth...>,
                             sub_precision<SubPrecision...>, sub_additional<C>>>
    : integer_base<C> {
    static_assert(sizeof(Integer) <= 8,
                  "general integer formatter does not support integer larger "
                  "than std::size_t (8 bytes)");

    using sub_flags_type = sub_flags<SubFlags...>;
    using sub_width_type = sub_width<SubWidth...>;

    template <Integer N, std::size_t R> constexpr static std::size_t log() {
        if constexpr (N != 0) {
            return log<N / integer_base<C>::n1(), R + 1>();
        } else {
            return R;
        }
    }

    constexpr static std::size_t max_digits_impl() {
        constexpr std::size_t raw_digits =
            log<std::numeric_limits<Integer>::max(), 0>();
        if constexpr (std::is_signed_v<Integer> ||
                      sub_flags_type::forced_sign) {
            return raw_digits + 1;
        } else {
            return raw_digits;
        }
    }

    template <std::size_t N, std::size_t R>
    constexpr static std::size_t ceil_digit() {
        if constexpr (N <= integer_base<C>::n2()) {
            return R + 1;
        } else {
            return ceil_digit<N - integer_base<C>::n2(), R + 1>();
        }
    }
    template <typename> struct ceil_digit_impl;
    template <std::size_t... Is>
    struct ceil_digit_impl<std::integer_sequence<std::size_t, Is...>> {
        constexpr static std::size_t table[65] = {ceil_digit<Is, 0>()...};
    };

    template <typename> struct ceil_digit_impl_row;
    template <std::size_t... Is>
    struct ceil_digit_impl_row<std::integer_sequence<std::size_t, Is...>> {};

    constexpr static std::size_t fast_digit_count(Integer i) noexcept(true) {
        if constexpr (std::is_signed_v<Integer>) {
            if (i != std::numeric_limits<Integer>::min()) {
                if (i < 0) {
                    return fast_digit_count(-i) + 1;
                }
            } else {
                return max_digits;
            }
        }
        return ceil_digit_impl<
            decltype(std::make_integer_sequence<std::size_t, 65>())>::table
            [64 - __builtin_clzll(i | 1)];
    }

    constexpr static inline std::size_t max_digits = max_digits_impl();

    constexpr static std::size_t max_size() noexcept(true) {
        if constexpr (!sub_width_type::ast) {
            if constexpr (sub_width_type::norm) {
                if constexpr (sub_width_type().to_number() > max_digits) {
                    return sub_width_type().to_number();
                } else {
                    return max_digits;
                }
            } else {
                return max_digits;
            }
        } else {
            return 0;
        }
    }

    constexpr static auto calculate_size(Integer i) noexcept(true) {
        if constexpr (sub_width_type::norm && !sub_flags_type::left_justify) {
            struct result_type {
                std::size_t full_size;
                std::size_t actual_size;
            } res = {};
            res.actual_size = fast_digit_count(i);
            if constexpr (sub_flags_type::forced_sign) {
                if (i >= 0) {
                    ++res.actual_size;
                }
            }
            res.full_size = res.actual_size >= sub_width_type().to_number()
                                ? res.actual_size
                                : sub_width_type().to_number();
            return res;
        } else {
            std::size_t res = fast_digit_count(i);
            if constexpr (sub_flags_type::forced_sign) {
                if (i >= 0) {
                    ++res;
                }
            }
            if constexpr (sub_flags_type::left_justify &&
                          sub_width_type::norm) {
                return std::max(res, sub_width_type().to_number());
            } else {
                return res;
            }
        }
    }

    template <typename RandomAccessIterator, typename SizeType>
    constexpr static std::size_t format(RandomAccessIterator ptr, Integer x,
                                        const SizeType& res) noexcept(true) {
        if constexpr (sub_width_type::norm && !sub_flags_type::left_justify) {
            // the only right-justify branch
            if (res.actual_size != res.full_size) {
                for (std::size_t offset = res.full_size - res.actual_size;
                     offset != 0; --offset) {
                    if constexpr (sub_flags_type::zero_padding) {
                        *(ptr++) = '0';
                    } else {
                        *(ptr++) = ' ';
                    }
                }
            }
            final_format(x, ptr, res.actual_size);
            return res.full_size;
        }  // if(ast and not left-justify) {}
        else {
            final_format(x, ptr, res);
            return res;
        }
    }
    template <typename RandomAccessIterator>
    static void final_format(Integer x, RandomAccessIterator ptr,
                             std::size_t len) noexcept(true) {
        if constexpr (sub_flags_type::forced_sign) {
            if (x >= 0) {
                *(ptr++) = '+';
                --len;
            }
        }
        std::to_chars_result r =
            std::to_chars(detail::iter_addr(ptr), detail::iter_addr(ptr + len),
                          x, integer_base<C>::n1());
        if constexpr (sub_flags_type::left_justify) {
            const std::size_t num_size = r.ptr - detail::iter_addr(ptr);
            for (len -= num_size, ptr += num_size; len != 0; --len) {
                *(ptr++) = ' ';
            }
        }
    }
};

template <typename Integer, char... SubFlags, char... SubWidth,
          char... SubPrecision, char... SubAdditional>
struct integer_general<
    Integer,
    flags<sub_flags<SubFlags...>, sub_width<SubWidth...>,
          sub_precision<SubPrecision...>, sub_additional<SubAdditional...>>> {
    static_assert(sizeof(Integer) <= 8,
                  "general integer formatter does not support integer larger "
                  "than std::size_t (8 bytes)");

    using sub_flags_type = sub_flags<SubFlags...>;
    using sub_width_type = sub_width<SubWidth...>;

    template <Integer Number, std::size_t R>
    constexpr static std::size_t max_digits_impl() {
        if constexpr (Number != 0) {
            return max_digits_impl<Number / 10, R + 1>();
        } else {
            if constexpr (std::is_signed_v<Integer> ||
                          sub_flags_type::forced_sign) {
                return R + 1;
            } else {
                return R;
            }
        }
    }

    constexpr static inline std::size_t max_digits =
        max_digits_impl<std::numeric_limits<Integer>::max(), 0>();

    struct fast_digit_count2 {
        template <std::size_t I, std::size_t R = 1>
        constexpr static std::size_t digit_n() {
            if constexpr (I / 10 != 0) {
                return digit_n<I / 10, R + 1>();
            } else {
                return R;
            }
        }
        template <std::size_t N> constexpr static std::size_t multi_9() {
            if constexpr (N == 19) {
                return 9999999999999999999u;
            }
            if constexpr (N == 20) {
                return -1;
            }
            if constexpr (N != 0) {
                return 9 * pow<10, N>() + multi_9<N - 1>();
            } else {
                return 9;
            }
        }

        template <std::size_t A, std::size_t B>
        constexpr static std::size_t min() {
            if constexpr (A < B) {
                return A;
            } else {
                return B;
            }
        }

        template <std::size_t A, std::size_t B>
        constexpr static std::size_t auto_choice() {
            if constexpr (A == 64) {
                return 20;
            }
            constexpr std::size_t pow_A = pow<2, A>();
            constexpr std::size_t N = digit_n<pow_A>();
            constexpr std::size_t int_9 = multi_9<B>();
            if constexpr (pow_A > int_9) {
                return N - 1;
            } else {
                return N;
            }
        }
        template <std::size_t... Is> struct table_t {
            constexpr static std::size_t table[sizeof...(Is)][2] = {
                {pow<2, Is>(),
                 min<pow<2, Is + 1>(),
                     multi_9<digit_n<pow<2, Is>()>() - 1>()>()}...};
            constexpr static std::size_t result[sizeof...(Is)][2] = {
                {digit_n<pow<2, Is>()>(),
                 auto_choice<Is + 1, digit_n<pow<2, Is>()>()>()}...};
        };
        template <std::size_t... Is>
        static constexpr table_t<Is...>
        create_table(std::integer_sequence<std::size_t, Is...>) {
            return {};
        }
        constexpr static std::size_t f(Integer i) noexcept(true) {
            if constexpr (std::is_signed_v<Integer>) {
                if (i != std::numeric_limits<Integer>::min()) {
                    if (i < 0) {
                        return f(-i) + 1;
                    }
                } else {
                    return max_digits;
                }
            }
            constexpr auto t =
                create_table(std::make_integer_sequence<std::size_t, 64>{});
            std::size_t __log2 = 63 - __builtin_clzll(i | 1);
            return t.result[__log2][i > t.table[__log2][1]];
        }
    };

    constexpr static std::size_t max_size() noexcept(true) {
        if constexpr (!sub_width_type::ast) {
            if constexpr (sub_width_type::norm) {
                if constexpr (sub_width_type().to_number() > max_digits) {
                    return sub_width_type().to_number();
                } else {
                    return max_digits;
                }
            } else {
                return max_digits;
            }
        } else {
            return 0;
        }
    }

    constexpr static auto calculate_size(Integer i) noexcept(true) {
        if constexpr (sub_width_type::norm && !sub_flags_type::left_justify) {
            // the only right-justify branch
            struct result_type {
                std::size_t full_size;
                std::size_t actual_size;
            } res = {};
            res.actual_size = fast_digit_count2::f(i);
            if constexpr (sub_flags_type::forced_sign) {
                if (i >= 0) {
                    res.actual_size += 1;
                }
            }
            res.full_size = res.actual_size >= sub_width_type().to_number()
                                ? res.actual_size
                                : sub_width_type().to_number();
            return res;
        } else {
            std::size_t res = fast_digit_count2::f(i);
            if constexpr (sub_flags_type::forced_sign) {
                if (i >= 0) {
                    res += 1;
                }
            }
            if constexpr (sub_flags_type::left_justify &&
                          sub_width_type::norm) {
                return std::max(res, sub_width_type().to_number());
            } else {
                return res;
            }
        }
    }

    template <typename RandomAccessIterator, typename SizeType>
    constexpr static std::size_t format(RandomAccessIterator ptr, Integer x,
                                        const SizeType& res) noexcept(true) {
        if constexpr (sub_width_type::norm && !sub_flags_type::left_justify) {
            // the only right-justify branch
            if (res.actual_size != res.full_size) {
                for (std::size_t offset = res.full_size - res.actual_size;
                     offset != 0; --offset) {
                    if constexpr (sub_flags_type::zero_padding) {
                        *(ptr++) = '0';
                    } else {
                        *(ptr++) = ' ';
                    }
                }
            }
            final_format(x, ptr, res.actual_size);
            return res.full_size;
        }  // if(ast and not left-justify) {}
        else {
            final_format(x, ptr, res);
            return res;
        }
    }
    template <typename RandomAccessIterator>
    static void final_format(Integer x, RandomAccessIterator ptr,
                             std::size_t len) noexcept(true) {
        if constexpr (sub_flags_type::forced_sign) {
            if (x >= 0) {
                *(ptr++) = '+';
                --len;
            }
        }
        std::to_chars_result r = std::to_chars(detail::iter_addr(ptr),
                                               detail::iter_addr(ptr + len), x);
        if constexpr (sub_flags_type::left_justify) {
            const std::size_t num_size = r.ptr - detail::iter_addr(ptr);
            for (len -= num_size, ptr += num_size; len != 0; --len) {
                *(ptr++) = ' ';
            }
        }
    }
};
}  // namespace detail

template <typename... SubSpecifiers>
struct formatter<flags<SubSpecifiers...>, specifier<'c'>> {
    struct calculated_size_type {
        static constexpr inline std::size_t full_size = 1;
    };
    constexpr static std::size_t max_size() noexcept(true) { return 1; }
    constexpr static calculated_size_type calculate_size(char) noexcept(true) {
        return {};
    }
    template <typename RandomAccessIterator>
    constexpr static std::size_t format(RandomAccessIterator ptr, char x,
                                        calculated_size_type) noexcept(true) {
        *ptr = x;
        return 1;
    }
};

template <char... SubFlags, char... SubWidth, char... SubPrecision,
          char... SubAdditional>
struct formatter<
    flags<sub_flags<SubFlags...>, sub_width<SubWidth...>,
          sub_precision<SubPrecision...>, sub_additional<SubAdditional...>>,
    specifier<'d'>>
    : detail::integer_general<
          int, flags<sub_flags<SubFlags...>, sub_width<SubWidth...>,
                     sub_precision<SubPrecision...>,
                     sub_additional<SubAdditional...>>> {};
template <char... SubFlags, char... SubWidth, char... SubPrecision,
          char... SubAdditional>
struct formatter<
    flags<sub_flags<SubFlags...>, sub_width<SubWidth...>,
          sub_precision<SubPrecision...>, sub_additional<SubAdditional...>>,
    specifier<'u'>>
    : detail::integer_general<
          unsigned int, flags<sub_flags<SubFlags...>, sub_width<SubWidth...>,
                              sub_precision<SubPrecision...>,
                              sub_additional<SubAdditional...>>> {};
template <char... SubFlags, char... SubWidth, char... SubPrecision,
          char... SubAdditional>
struct formatter<
    flags<sub_flags<SubFlags...>, sub_width<SubWidth...>,
          sub_precision<SubPrecision...>, sub_additional<SubAdditional...>>,
    specifier<'l', 'd'>>
    : detail::integer_general<
          long, flags<sub_flags<SubFlags...>, sub_width<SubWidth...>,
                      sub_precision<SubPrecision...>,
                      sub_additional<SubAdditional...>>> {};
template <char... SubFlags, char... SubWidth, char... SubPrecision,
          char... SubAdditional>
struct formatter<
    flags<sub_flags<SubFlags...>, sub_width<SubWidth...>,
          sub_precision<SubPrecision...>, sub_additional<SubAdditional...>>,
    specifier<'l', 'u'>>
    : detail::integer_general<
          std::size_t, flags<sub_flags<SubFlags...>, sub_width<SubWidth...>,
                             sub_precision<SubPrecision...>,
                             sub_additional<SubAdditional...>>> {};
}  // namespace chx::log
