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

struct hmsdm_table_t {
    template <std::size_t N>
    constexpr static string<N / 10 + '0', N % 10 + '0'>
    to_string() noexcept(true) {
        return {};
    }
    template <std::size_t... Is>
    constexpr void make_h(std::integer_sequence<std::size_t, Is...>) {
        (to_string<Is>().copy_to(table[Is]), ...);
    }
    template <std::size_t N> constexpr static auto make_space_pad() {
        if constexpr (N < 10) {
            return string<' ', N + '0'>{};
        } else {
            return to_string<N>();
        }
    }
    template <std::size_t... Is>
    constexpr void make_s(std::integer_sequence<std::size_t, Is...>) {
        (to_string<Is>().copy_to(table[Is]), ...);
    }

    constexpr hmsdm_table_t() noexcept(true) {
        make_h(std::make_integer_sequence<std::size_t, 100>{});
        make_s(std::make_integer_sequence<std::size_t, 100>{});
    }

    char table[100][2] = {};
    char stable[100][2] = {};
};
constexpr hmsdm_table_t hmsdm_table{};

// https://en.wikipedia.org/wiki/ISO_week_date
constexpr std::size_t wpy_p(int y) noexcept(true) {
    return y + y * 4 - y / 100 + y / 400;
}
constexpr std::size_t wpy(int y) noexcept(true) {
    return 52 + ((wpy_p(y) == 4 || wpy_p(y - 1) == 3) ? 1 : 0);
}
constexpr int wby(const std::tm& t) noexcept(true) {
    int y = t.tm_year + 1900;
    int w = (11 + t.tm_yday - (t.tm_wday == 0 ? 7 : t.tm_wday)) / 7;
    if (w == 0) {
        return y - 1;
    } else if (w > wpy(y)) {
        return y + 1;
    } else {
        return y;
    }
}
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
        ptr =
            std::copy_n(detail::chrono::hmsdm_table.table[_tm.tm_mday], 2, ptr);
        *(ptr++) = ' ';
        ptr =
            std::copy_n(detail::chrono::hmsdm_table.table[_tm.tm_hour], 2, ptr);
        *(ptr++) = ':';
        ptr =
            std::copy_n(detail::chrono::hmsdm_table.table[_tm.tm_min], 2, ptr);
        *(ptr++) = ':';
        ptr =
            std::copy_n(detail::chrono::hmsdm_table.table[_tm.tm_sec], 2, ptr);
        *(ptr++) = ' ';
        int y = (_tm.tm_year + 1900) % 10000;
        ptr = std::copy_n(detail::chrono::hmsdm_table.table[y / 100], 2, ptr);
        ptr = std::copy_n(detail::chrono::hmsdm_table.table[y % 100], 2, ptr);
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
        std::copy_n(detail::chrono::hmsdm_table.table[_tm.tm_year / 100 + 19],
                    2, iter);
    }
};

template <> struct chrono_subformatter<flags<>, specifier<'d'>> {
    constexpr static std::size_t max_size() noexcept(true) { return 2; }
    constexpr static std::size_t
    calculate_size(const struct std::tm& t) noexcept(true) {
        return 2;
    }
    template <typename RandomAccessIterator>
    constexpr static void format(RandomAccessIterator iter,
                                 const struct std::tm& _tm,
                                 std::size_t) noexcept(true) {
        std::copy_n(detail::chrono::hmsdm_table.table[_tm.tm_mday], 2, iter);
    }
};

template <> struct chrono_subformatter<flags<>, specifier<'D'>> {
    constexpr static std::size_t max_size() noexcept(true) { return 8; }
    constexpr static std::size_t
    calculate_size(const struct std::tm& t) noexcept(true) {
        return 8;
    }
    template <typename RandomAccessIterator>
    constexpr static void format(RandomAccessIterator iter,
                                 const struct std::tm& _tm,
                                 std::size_t) noexcept(true) {
        // mdy
        char* ptr =
            std::copy_n(detail::chrono::hmsdm_table.table[_tm.tm_mon + 1], 2,
                        detail::iter_addr(iter));
        *(ptr++) = '/';
        ptr =
            std::copy_n(detail::chrono::hmsdm_table.table[_tm.tm_mday], 2, ptr);
        *(ptr++) = '/';
        std::copy_n(detail::chrono::hmsdm_table.table[_tm.tm_year % 100], 2,
                    ptr);
    }
};

template <> struct chrono_subformatter<flags<>, specifier<'e'>> {
    constexpr static std::size_t max_size() noexcept(true) { return 2; }
    constexpr static std::size_t
    calculate_size(const struct std::tm& t) noexcept(true) {
        return 2;
    }
    template <typename RandomAccessIterator>
    constexpr static void format(RandomAccessIterator iter,
                                 const struct std::tm& _tm,
                                 std::size_t) noexcept(true) {
        std::copy_n(detail::chrono::hmsdm_table.stable[_tm.tm_mday], 2, iter);
    }
};

template <> struct chrono_subformatter<flags<>, specifier<'F'>> {
    constexpr static std::size_t max_size() noexcept(true) { return 10; }
    constexpr static std::size_t
    calculate_size(const struct std::tm& t) noexcept(true) {
        return 10;
    }
    template <typename RandomAccessIterator>
    constexpr static void format(RandomAccessIterator iter,
                                 const struct std::tm& _tm,
                                 std::size_t) noexcept(true) {
        int y = (_tm.tm_year + 1900) % 10000;
        char* ptr = std::copy_n(detail::chrono::hmsdm_table.table[y / 100], 2,
                                detail::iter_addr(iter));
        ptr = std::copy_n(detail::chrono::hmsdm_table.table[y % 100], 2, ptr);
        *(ptr++) = '-';
        ptr = std::copy_n(detail::chrono::hmsdm_table.table[_tm.tm_mon + 1], 2,
                          ptr);
        *(ptr++) = '-';
        std::copy_n(detail::chrono::hmsdm_table.table[_tm.tm_mday], 2, ptr);
    }
};

template <> struct chrono_subformatter<flags<>, specifier<'g'>> {
    constexpr static std::size_t max_size() noexcept(true) { return 2; }
    constexpr static std::size_t
    calculate_size(const struct std::tm& t) noexcept(true) {
        return 2;
    }
    template <typename RandomAccessIterator>
    constexpr static void format(RandomAccessIterator iter,
                                 const struct std::tm& _tm,
                                 std::size_t) noexcept(true) {
        int wby = detail::chrono::wby(_tm) % 100;
        std::copy_n(detail::chrono::hmsdm_table.table[wby], 2, iter);
    }
};

template <> struct chrono_subformatter<flags<>, specifier<'G'>> {
    constexpr static std::size_t max_size() noexcept(true) { return 4; }
    constexpr static std::size_t
    calculate_size(const struct std::tm& t) noexcept(true) {
        return 4;
    }
    template <typename RandomAccessIterator>
    constexpr static void format(RandomAccessIterator iter,
                                 const struct std::tm& _tm,
                                 std::size_t) noexcept(true) {
        int wby = detail::chrono::wby(_tm) % 10000;
        auto p =
            std::copy_n(detail::chrono::hmsdm_table.table[wby / 100], 2, iter);
        std::copy_n(detail::chrono::hmsdm_table.table[wby % 100], 2, p);
    }
};

template <>
struct chrono_subformatter<flags<>, specifier<'h'>>
    : chrono_subformatter<flags<>, specifier<'b'>> {};

template <> struct chrono_subformatter<flags<>, specifier<'H'>> {
    constexpr static std::size_t max_size() noexcept(true) { return 2; }
    constexpr static std::size_t
    calculate_size(const struct std::tm& t) noexcept(true) {
        return 2;
    }
    template <typename RandomAccessIterator>
    constexpr static void format(RandomAccessIterator iter,
                                 const struct std::tm& _tm,
                                 std::size_t) noexcept(true) {
        std::copy_n(detail::chrono::hmsdm_table.table[_tm.tm_hour], 2, iter);
    }
};

template <> struct chrono_subformatter<flags<>, specifier<'I'>> {
    constexpr static std::size_t max_size() noexcept(true) { return 2; }
    constexpr static std::size_t
    calculate_size(const struct std::tm& t) noexcept(true) {
        return 2;
    }
    template <typename RandomAccessIterator>
    constexpr static void format(RandomAccessIterator iter,
                                 const struct std::tm& _tm,
                                 std::size_t) noexcept(true) {
        int h = _tm.tm_hour - (_tm.tm_hour > 12 ? 12 : 0);
        std::copy_n(detail::chrono::hmsdm_table.table[h ? h : 1], 2, iter);
    }
};

template <> struct chrono_subformatter<flags<>, specifier<'j'>> {
    constexpr static std::size_t max_size() noexcept(true) { return 2; }
    constexpr static std::size_t
    calculate_size(const struct std::tm& t) noexcept(true) {
        return 2;
    }
    template <typename RandomAccessIterator>
    constexpr static void format(RandomAccessIterator iter,
                                 const struct std::tm& _tm,
                                 std::size_t) noexcept(true) {
        std::copy_n(detail::chrono::hmsdm_table.table[_tm.tm_yday + 1], 2,
                    iter);
    }
};

template <> struct chrono_subformatter<flags<>, specifier<'m'>> {
    constexpr static std::size_t max_size() noexcept(true) { return 2; }
    constexpr static std::size_t
    calculate_size(const struct std::tm& t) noexcept(true) {
        return 2;
    }
    template <typename RandomAccessIterator>
    constexpr static void format(RandomAccessIterator iter,
                                 const struct std::tm& _tm,
                                 std::size_t) noexcept(true) {
        std::copy_n(detail::chrono::hmsdm_table.table[_tm.tm_mon + 1], 2, iter);
    }
};

template <> struct chrono_subformatter<flags<>, specifier<'M'>> {
    constexpr static std::size_t max_size() noexcept(true) { return 2; }
    constexpr static std::size_t
    calculate_size(const struct std::tm& t) noexcept(true) {
        return 2;
    }
    template <typename RandomAccessIterator>
    constexpr static void format(RandomAccessIterator iter,
                                 const struct std::tm& _tm,
                                 std::size_t) noexcept(true) {
        std::copy_n(detail::chrono::hmsdm_table.table[_tm.tm_min], 2, iter);
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
