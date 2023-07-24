#pragma once

#include "./format.hpp"

#include <chrono>
#include <locale>
#include <charconv>

namespace chx::log {
namespace detail::chrono {
constexpr static inline char wday_name_abbr[7][4] = {"Sun", "Mon", "Tue", "Wed",
                                                     "Thu", "Fri", "Sat"};
constexpr static inline char wday_name[7][16] = {
    "Sunday",   "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"};
constexpr static inline unsigned wday_name_len[7] = {6, 6, 7, 9, 8, 6, 8};
constexpr static inline char mon_name_abbr[12][4] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
constexpr static inline char mom_name[12][16] = {
    "January", "February", "March",     "April",   "May",      "June",
    "July",    "August",   "September", "October", "November", "December"};
constexpr static inline unsigned mon_name_len[12] = {7, 8, 5, 5, 3, 4,
                                                     4, 6, 9, 7, 8, 8};

constexpr static inline std::tuple<
    string<'S', 'u', 'n', ' '>, string<'M', 'o', 'n', ' '>,
    string<'T', 'u', 'e', ' '>, string<'W', 'e', 'd', ' '>,
    string<'T', 'h', 'u', ' '>, string<'F', 'r', 'i', ' '>,
    string<'S', 'a', 't', ' '>>
    tp_wday;
constexpr static inline std::tuple<
    string<'J', 'a', 'n'>, string<'F', 'e', 'b'>, string<'M', 'a', 'r'>,
    string<'A', 'p', 'r'>, string<'M', 'a', 'y'>, string<'J', 'u', 'n'>,
    string<'J', 'u', 'l'>, string<'A', 'u', 'g'>, string<'S', 'e', 'p'>,
    string<'O', 'c', 't'>, string<'N', 'o', 'v'>, string<'D', 'e', 'c'>>
    tp_mon;

struct wday_mon_table_t {
    char table[7][12][8] = {};

    constexpr wday_mon_table_t() { fill(); }
    template <std::size_t... Is>
    constexpr void fill1(std::integer_sequence<std::size_t, Is...>) {
        (fill2<Is>(std::make_integer_sequence<std::size_t, 12>{}), ...);
    }
    template <std::size_t I, std::size_t... Js>
    constexpr void fill2(std::integer_sequence<std::size_t, Js...>) {
        (concat(std::get<I>(tp_wday), std::get<Js>(tp_mon))
             .copy_to(table[I][Js]),
         ...);
    }
    constexpr void fill() {
        fill1(std::make_integer_sequence<std::size_t, 7>{});
    }
};
constexpr static wday_mon_table_t wday_mon_table{};
}  // namespace detail::chrono
template <typename Flags, typename Specifier>
struct chrono_subformatter : formatter_placeholder {};

template <> struct chrono_subformatter<flags<>, specifier<'%'>> {
    struct size_result {
        constexpr static inline std::size_t full_size = 1;
    };
    constexpr static std::size_t max_size() noexcept(true) { return 1; }
    constexpr static size_result calculate_size() noexcept(true) { return {}; }
    template <typename RandomAccessIterator>
    constexpr void format(RandomAccessIterator iter, size_result) {
        *iter = '%';
    }
};

template <> struct chrono_subformatter<flags<>, specifier<'a'>> {
    constexpr static std::size_t max_size() noexcept(true) { return 3; }
    constexpr static std::size_t
    calculate_size(const struct std::tm&) noexcept(true) {
        return 3;
    }
    template <typename RandomAccessIterator>
    constexpr static std::size_t format(RandomAccessIterator iter,
                                        const struct std::tm& _tm,
                                        std::size_t) noexcept(true) {
        std::copy_n(detail::chrono::wday_name_abbr[_tm.tm_wday], 3, iter);
        return 3;
    }
};

template <> struct chrono_subformatter<flags<>, specifier<'A'>> {
    constexpr static std::size_t max_size() noexcept(true) { return 9; }
    constexpr static std::size_t
    calculate_size(const struct std::tm& t) noexcept(true) {
        return detail::chrono::wday_name_len[t.tm_wday];
    }
    template <typename RandomAccessIterator>
    constexpr static void format(RandomAccessIterator iter,
                                 const struct std::tm& _tm,
                                 std::size_t) noexcept(true) {
        std::copy_n(detail::chrono::wday_name[_tm.tm_wday],
                    detail::chrono::wday_name_len[_tm.tm_wday], iter);
    }
};

template <> struct chrono_subformatter<flags<>, specifier<'b'>> {
    constexpr static std::size_t max_size() noexcept(true) { return 3; }
    constexpr static std::size_t
    calculate_size(const struct std::tm&) noexcept(true) {
        return 3;
    }
    template <typename RandomAccessIterator>
    constexpr static std::size_t format(RandomAccessIterator iter,
                                        const struct std::tm& _tm,
                                        std::size_t) noexcept(true) {
        std::copy_n(detail::chrono::mon_name_abbr[_tm.tm_mon], 3, iter);
        return 3;
    }
};

template <> struct chrono_subformatter<flags<>, specifier<'B'>> {
    constexpr static std::size_t max_size() noexcept(true) { return 9; }
    constexpr static std::size_t
    calculate_size(const struct std::tm& t) noexcept(true) {
        return detail::chrono::mon_name_len[t.tm_wday];
    }
    template <typename RandomAccessIterator>
    constexpr static void format(RandomAccessIterator iter,
                                 const struct std::tm& _tm,
                                 std::size_t) noexcept(true) {
        std::copy_n(detail::chrono::mom_name[_tm.tm_wday],
                    detail::chrono::mon_name_len[_tm.tm_wday], iter);
    }
};

template <> struct chrono_subformatter<flags<>, specifier<'c'>> {
    // another asctime
    constexpr static std::size_t max_size() noexcept(true) { return 24; }
    constexpr static std::size_t
    calculate_size(const struct std::tm&) noexcept(true) {
        return 24;
    }

    template <typename RandomAccessIterator>
    constexpr static std::size_t format(RandomAccessIterator iter,
                                        const struct std::tm& _tm,
                                        std::size_t) noexcept(true) {
        char* ptr = detail::iter_addr(iter);
        ptr = std::copy_n(
            detail::chrono::wday_mon_table.table[_tm.tm_wday][_tm.tm_mon], 7,
            ptr);
        *(ptr++) = ' ';
        if (__builtin_expect(_tm.tm_mday < 10, 0)) {
            *(ptr++) = '0';
        }
        ptr = std::to_chars(ptr, ptr + 2, _tm.tm_mday).ptr;
        *(ptr++) = ' ';
        if (__builtin_expect(_tm.tm_hour < 10, 0)) {
            *(ptr++) = '0';
        }
        ptr = std::to_chars(ptr, ptr + 2, _tm.tm_hour).ptr;
        *(ptr++) = ':';
        if (__builtin_expect(_tm.tm_min < 10, 0)) {
            *(ptr++) = '0';
        }
        ptr = std::to_chars(ptr, ptr + 2, _tm.tm_min).ptr;
        *(ptr++) = ':';
        if (__builtin_expect(_tm.tm_sec < 10, 0)) {
            *(ptr++) = '0';
        }
        ptr = std::to_chars(ptr, ptr + 2, _tm.tm_sec).ptr;
        *(ptr++) = ' ';
        std::to_chars(ptr, ptr + 4, _tm.tm_year + 1900);
        return 24;
    }
};

template <> struct chrono_subformatter<flags<>, specifier<'C'>> {
    constexpr static std::size_t max_size() noexcept(true) { return 2; }
    constexpr static std::size_t
    calculate_size(const struct std::tm& t) noexcept(true) {
        return 2;
    }
    template <typename RandomAccessIterator>
    constexpr static void format(RandomAccessIterator iter,
                                 const struct std::tm& _tm,
                                 std::size_t) noexcept(true) {
        std::to_chars(detail::iter_addr(iter), detail::iter_addr(iter) + 2,
                      _tm.tm_year / 100 + 19);
    }
};

namespace detail {
struct chrono_subformatter_lookup {
    template <typename Flags, typename Specifier>
    constexpr static bool is_valid() noexcept(true) {
        return !std::is_base_of_v<formatter_placeholder,
                                  chrono_subformatter<Flags, Specifier>>;
    }
    template <typename Flags, typename Specifier>
    using rebind = chrono_subformatter<Flags, Specifier>;
};

template <std::size_t... Is, typename Tp>
constexpr auto deduce_size_type(Tp& tp,
                                std::integer_sequence<std::size_t, Is...>) {
    return std::make_tuple(
        std::get<Is>(tp).formatter.calculate_size(std::tm{})...);
}
}  // namespace detail

template <char... SubFlags, char... SubWidth, char... SubPrecision>
struct formatter<flags<sub_flags<SubFlags...>, sub_width<SubWidth...>,
                       sub_precision<SubPrecision...>, sub_additional<>>,
                 specifier<'C'>>
    : chrono_subformatter<flags<>, specifier<'c'>> {};

template <char... SubFlags, char... SubWidth, char... SubPrecision,
          char... SubFormatter>
struct formatter<
    flags<sub_flags<SubFlags...>, sub_width<SubWidth...>,
          sub_precision<SubPrecision...>, sub_additional<SubFormatter...>>,
    specifier<'C'>>
    : detail::core2<detail::chrono_subformatter_lookup>::subformatter_helper<
          formatter<flags<sub_flags<SubFlags...>, sub_width<SubWidth...>,
                          sub_precision<SubPrecision...>,
                          sub_additional<SubFormatter...>>,
                    specifier<'C'>>,
          struct std::tm, SubFormatter...> {
    struct no_tm_result {
        std::tm t = {};
        std::size_t full_size = 0;
    };
    using _h = typename formatter::helper_type;
    using _h::calculate_size;
    using _h::format;

    template <typename Clock, typename Duration>
    no_tm_result calculate_size(
        const std::chrono::time_point<Clock, Duration>& tp) noexcept(true) {
        no_tm_result r = {};
        std::time_t tt = Clock::to_time_t(tp);
        localtime_r(&tt, &r.t);
        r.full_size = _h::calculate_size(r.t);
        return std::move(r);
    }
    template <typename RandomAccessIterator, typename Clock, typename Duration>
    constexpr void format(RandomAccessIterator iter,
                          const std::chrono::time_point<Clock, Duration>& tp,
                          const no_tm_result& s) noexcept(true) {
        _h::format(iter, s.t, s.full_size);
    }
};
}  // namespace chx::log
