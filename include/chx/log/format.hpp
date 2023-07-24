#pragma once

#include "./carrier.hpp"
#include "./detail/constexpr_string.hpp"
#include "./detail/iterator_address.hpp"

#include <algorithm>
#include <string>

namespace chx::log {
namespace detail {
template <typename R> struct is_invocable_calculate_impl {
    template <typename... T,
              typename = decltype(std::declval<R>().calculate_size(
                  std::declval<T>()...))>
    is_invocable_calculate_impl(T&&...) {}
};
template <typename R> struct is_invocable_format_impl {
    template <typename... T, typename = decltype(std::declval<R>().format(
                                 std::declval<T>()...))>
    is_invocable_format_impl(T&&...) {}
};

template <typename T, typename R, typename N, typename I1, typename I2>
using can_calculate_with =
    std::is_constructible<is_invocable_calculate_impl<T>, R, N, I1, I2>;
template <typename T, typename... R>
using can_format_with =
    std::is_constructible<is_invocable_format_impl<T>, R...>;
}  // namespace detail

template <std::size_t Idx, std::size_t ArgPos> struct formatter_context {
    static constexpr std::size_t index = Idx;
    using index_t = std::integral_constant<std::size_t, index>;
    static constexpr std::size_t arg_pos = ArgPos;
    using arg_pos_t = std::integral_constant<std::size_t, arg_pos>;

    template <typename T, typename ValueRef, typename Rtp>
    constexpr static decltype(auto)
    invoke_calculate_size(T&& t, ValueRef& value_ref, Rtp& rtp) {
        if constexpr (detail::can_calculate_with<std::decay_t<T>, ValueRef&,
                                                 Rtp&, index_t,
                                                 arg_pos_t>::value) {
            return t.calculate_size(value_ref, rtp, index_t{}, arg_pos_t{});
        } else {
            return t.calculate_size(value_ref);
        }
    }
    template <typename T, typename ValueRef, typename Rtp,
              typename RandomAccessIterator, typename Size>
    constexpr static void invoke_format(T&& t, ValueRef& value_ref, Rtp& rtp,
                                        RandomAccessIterator& iter,
                                        Size& size) {
        if constexpr (detail::can_format_with<
                          std::decay_t<T>, RandomAccessIterator&, ValueRef&,
                          Rtp&, index_t, arg_pos_t, Size&>::value) {
            t.format(iter, value_ref, rtp, index_t{}, arg_pos_t{}, size);
        } else {
            t.format(iter, value_ref, size);
        }
    }
};

template <char... C> struct sub_flags : string<C...> {
    static constexpr bool left_justify =
        string<C...>().template contains<'-'>();
    static constexpr bool forced_sign = string<C...>().template contains<'+'>();
    static constexpr bool space = string<C...>().template contains<' '>();
    static constexpr bool sharp = string<C...>().template contains<'#'>();
    static constexpr bool zero_padding =
        string<C...>().template contains<'0'>();
};
template <char... C> struct sub_width : string<C...> {
    static constexpr bool ast = (string<C...>() == string<'*'>());
    static constexpr bool norm =
        (string<C...>() != string<'*'>()) && (sizeof...(C) > 0);
    static constexpr bool empty = sizeof...(C) == 0;
};
template <char... C> struct sub_precision : string<C...> {};
template <char... C> struct sub_additional : string<C...> {};

template <typename SubFlag = sub_flags<>, typename SubWidth = sub_width<>,
          typename SubPrecision = sub_precision<>,
          typename SubAdditional = sub_additional<>>
struct flags {
    using flag_type = SubFlag;
    using width_type = SubWidth;
    using precision_type = SubPrecision;
    using additional_type = SubAdditional;
};
template <char... Cs> struct specifier : string<Cs...> {};

namespace detail {
struct bad_sub_specifier {};
using invalid_flags = flags<bad_sub_specifier, bad_sub_specifier,
                            bad_sub_specifier, bad_sub_specifier>;
template <char C> constexpr bool is_digit() { return C >= '0' && C <= '9'; }

struct flag_specifiers {
    template <std::size_t Begin, std::size_t End, std::size_t Type,
              typename SubSpecifier>
    struct parse_result {
        static constexpr inline std::size_t begin = Begin;
        static constexpr inline std::size_t end = End;
        static constexpr inline std::size_t type = Type;
        using sub_specifier_type = SubSpecifier;
    };

    struct additional_continue {};
    template <char... Cs> static constexpr auto parse(string<Cs...> str) {
        if constexpr (str.size == 0) {
            return flags<sub_flags<>, sub_width<>, sub_precision<>>{};
        } else {
            using sub_additional_result = decltype(parse_additional_begin(str));
            if constexpr (std::is_same_v<sub_additional_result,
                                         additional_continue>) {
                return additional_continue{};
            } else {
                constexpr auto str1 =
                    str.template sub<sub_additional_result::end,
                                     str.size - sub_additional_result::end>();
                using sub_flag_result =
                    decltype(parse_flags<1, parse_result<0, 0, 0, sub_flags<>>>(
                        str1));
                constexpr auto str2 =
                    str1.template sub<sub_flag_result::end,
                                      str1.size - sub_flag_result::end>();
                using sub_width_result = decltype(parse_width(str2));
                constexpr auto str3 =
                    str2.template sub<sub_width_result::end,
                                      str2.size - sub_width_result::end>();
                using sub_precision_result = decltype(parse_precision(str3));
                // static_assert(sub_precision_result::end == str3.size);
                if constexpr (sub_precision_result::end == str3.size) {
                    return flags<
                        typename sub_flag_result::sub_specifier_type,
                        typename sub_width_result::sub_specifier_type,
                        typename sub_precision_result::sub_specifier_type,
                        typename sub_additional_result::sub_specifier_type>{};
                } else {
                    return invalid_flags{};
                }
            }
        }
    }

    template <char... Cs>
    static constexpr sub_width<Cs...> to_width(string<Cs...>) {
        return {};
    }
    template <char... Cs>
    static constexpr sub_precision<Cs...> to_precision(string<Cs...>) {
        return {};
    }
    template <char... Cs>
    static constexpr sub_flags<Cs...> to_flags(string<Cs...>) {
        return {};
    }
    template <char... Cs>
    static constexpr sub_additional<Cs...> to_additional(string<Cs...>) {
        return {};
    }
    template <char... Cs>
    static constexpr auto parse_additional_begin(string<Cs...> str) {
        if constexpr (str.template get<0>() == ':') {
            constexpr std::size_t tail_colon = till_colon<1>(str);
            if constexpr (tail_colon == str.size) {
                return additional_continue{};
            } else {
                return parse_result<
                    1, tail_colon + 1, 3,
                    decltype(to_additional(
                        str.template sub<1, tail_colon - 1>()))>{};
            }
        } else {
            return parse_result<0, 0, 3, sub_additional<>>{};
        }
    }
    template <std::size_t Idx, char... Cs>
    static constexpr auto till_colon(string<Cs...> str) {
        if constexpr (Idx != str.size) {
            if constexpr (str.template get<Idx>() == ':') {
                return Idx;
            } else {
                return till_colon<Idx + 1>(str);
            }
        } else {
            return Idx;
        }
    }
    template <std::size_t End, typename Prev, char... Cs>
    static constexpr auto parse_flags(string<Cs...> str) {
        if constexpr (End <= sizeof...(Cs)) {
            constexpr char curr_char = str.template get<End - 1>();
            if constexpr (curr_char == '-' || curr_char == '+' ||
                          curr_char == ' ' || curr_char == '#' ||
                          curr_char == '0') {
                using current_section = parse_result<
                    Prev::begin, End, 0,
                    decltype(to_flags(
                        str.template sub<Prev::begin, End - Prev::begin>()))>;
                return parse_flags<End + 1, current_section>(str);
            } else {
                return Prev{};
            }
        } else {
            return Prev{};
        }
    }

    template <typename Str> using to_width_t = decltype(to_width(Str{}));
    template <typename Str>
    using to_precision_t = decltype(to_precision(Str{}));

    template <char... Cs> static constexpr auto parse_width(string<Cs...> str) {
        if constexpr (sizeof...(Cs) != 0) {
            if constexpr (str.template get<0>() == '*') {
                return parse_result<0, 1, 1, sub_width<'*'>>();
            } else {
                using failed_result = parse_result<0, 0, 1, sub_width<>>;
                return parse_width_digit<1, failed_result>(str);
            }
        } else {
            return parse_result<0, 0, 1, sub_width<>>{};
        }
    }
    template <std::size_t End, typename Prev, char... Cs>
    static constexpr auto parse_width_digit(string<Cs...> str) {
        if constexpr (End <= sizeof...(Cs)) {
            if constexpr (is_digit<str.template get<End - 1>()>()) {
                using current_section = parse_result<
                    Prev::begin, End, 1,
                    decltype(to_width(
                        str.template sub<Prev::begin, End - Prev::begin>()))>;
                return parse_width_digit<End + 1, current_section>(str);
            } else {
                return Prev{};
            }
        } else {
            return Prev{};
        }
    }
    template <char... Cs>
    static constexpr auto parse_precision(string<Cs...> str) {
        if constexpr (sizeof...(Cs) != 0) {
            if constexpr (str.template get<0>() == '.') {
                if constexpr (str.template get<1>() == '*') {
                    return parse_result<0, 2, 2, sub_precision<'*'>>{};
                } else {
                    return parse_precision_digit<
                        2, parse_result<0, 0, 2, sub_precision<>>>(str);
                }
            } else {
                return parse_result<0, 0, 2, sub_precision<>>{};
            }
        } else {
            return parse_result<0, 0, 2, sub_precision<>>{};
        }
    }
    template <std::size_t End, typename Prev, char... Cs>
    static constexpr auto parse_precision_digit(string<Cs...> str) {
        if constexpr (End <= sizeof...(Cs)) {
            if constexpr (is_digit<str.template get<End - 1>()>()) {
                using current_section = parse_result<
                    Prev::begin, End, 1,
                    decltype(to_precision(
                        str.template sub<Prev::begin + 1,
                                         End - Prev::begin - 1>()))>;
                return parse_precision_digit<End + 1, current_section>(str);
            } else {
                return Prev{};
            }
        } else {
            return Prev{};
        }
    }
};
}  // namespace detail

struct formatter_placeholder {};
template <typename Flags, typename Specifier>
struct formatter : formatter_placeholder {};

template <> struct formatter<flags<>, specifier<'%'>> {
    struct calculated_size {
        static constexpr inline std::size_t full_size = 1;
    };

    constexpr std::size_t max_size() noexcept(true) { return 1; }
    constexpr calculated_size calculate_size() noexcept(true) { return {}; }
    template <typename RandomAccessIterator>
    constexpr void format(RandomAccessIterator iter,
                          std::size_t) noexcept(true) {
        *iter = '%';
    }
};

namespace detail {
struct has_max_size_impl {
    template <typename R> static std::true_type f(decltype(&R::max_size));
    template <typename R> static std::false_type f(...);
};

template <typename T> using has_max_size = decltype(has_max_size_impl::f<T>(0));

struct default_formatter_lookup {
    template <typename Flags, typename Specifier>
    constexpr static bool is_valid() noexcept(true) {
        return !std::is_base_of_v<formatter_placeholder,
                                  formatter<Flags, Specifier>>;
    }
    template <typename Flags, typename Specifier>
    using rebind = formatter<Flags, Specifier>;
};
template <typename FormatterSpec = default_formatter_lookup> struct core2 {
    template <char... Cs> static constexpr auto to_flags(string<Cs...> str) {
        return flag_specifiers::parse(str);
    }
    template <char... Cs>
    static constexpr specifier<Cs...> to_specifier(string<Cs...>) {
        return {};
    }

    struct failed_result {};
    template <char... Cs>
    static constexpr auto construct_formatter(string<Cs...> str) {
        using result = decltype(construct_formatter_impl<0, failed_result>(
            str.template sub<1, str.size - 1>()));
        return result{};
    }
    template <std::size_t Idx, typename PrevFormatter, char... Cs>
    static constexpr auto construct_formatter_impl(string<Cs...> str) {
        if constexpr (Idx != sizeof...(Cs)) {
            using __flags_type = decltype(to_flags(str.template sub<0, Idx>()));
            using __specifier_type =
                decltype(to_specifier(str.template sub<Idx, str.size - Idx>()));
            static_assert(__specifier_type::size != 0);
            if constexpr (std::is_same_v<__flags_type, invalid_flags>) {
                return PrevFormatter{};
            } else if constexpr (FormatterSpec::template is_valid<
                                     __flags_type, __specifier_type>()) {
                return
                    typename FormatterSpec::template rebind<__flags_type,
                                                            __specifier_type>{};
            } else {
                return construct_formatter_impl<Idx + 1, PrevFormatter>(str);
            }
        } else {
            return PrevFormatter{};
        }
    }
    template <typename Result> static constexpr bool valid_construct() {
        return !std::is_same_v<typename Result::formatter_type, failed_result>;
    }

    template <std::size_t Begin, std::size_t End, std::size_t Type,
              typename Formatter>
    struct scan_result {
        constexpr static inline std::size_t begin = Begin;
        constexpr static inline std::size_t end = End;
        constexpr static inline std::size_t type = Type;
        using formatter_type = Formatter;
    };
    template <std::size_t Idx, std::size_t Size, typename Last, char... Cs>
    static constexpr auto scan_formatter0(string<Cs...> str) {
        if constexpr (Idx + Size <= str.size) {
            using curr_result =
                scan_result<Idx, Idx + Size, 0,
                            decltype(construct_formatter(
                                str.template sub<Idx, Size>()))>;
            if constexpr (valid_construct<curr_result>()) {
                return scan_formatter1<Idx, Size + 1, curr_result>(str);
            } else {
                return scan_formatter0<Idx, Size + 1, Last>(str);
            }
        } else {
            return Last{};
        }
    }
    template <std::size_t Idx, std::size_t Size, typename Prev, char... Cs>
    static constexpr auto scan_formatter1(string<Cs...> str) {
        if constexpr (Idx + Size <= str.size) {
            using curr_result =
                scan_result<Idx, Idx + Size, 0,
                            decltype(construct_formatter(
                                str.template sub<Idx, Size>()))>;
            if constexpr (valid_construct<curr_result>()) {
                return scan_formatter1<Idx, Size + 1, curr_result>(str);
            } else {
                return Prev{};
            }
        } else {
            return Prev{};
        }
    }
    template <char... Cs> struct norm_formatter {
        struct result_type {
            static constexpr inline std::size_t full_size = sizeof...(Cs);
        };

        constexpr std::size_t max_size() noexcept(true) {
            return sizeof...(Cs);
        }
        constexpr result_type calculate_size() noexcept(true) { return {}; }
        template <typename RandomAccessIterator, typename T>
        constexpr void format(RandomAccessIterator iter, T) noexcept(true) {
            std::copy_n(string<Cs...>::data, sizeof...(Cs), iter);
        }
    };
    template <char... Cs>
    static constexpr norm_formatter<Cs...> make_norm_formatter(string<Cs...>) {
        return {};
    }
    template <std::size_t Begin, std::size_t Idx, typename Prev, char... Cs>
    static constexpr auto scan_norm(string<Cs...> str) {
        if constexpr (Idx != sizeof...(Cs)) {
            if constexpr (str.template get<Idx>() != '%') {
                using cur_res = scan_result<
                    Begin, Idx + 1, 1,
                    decltype(make_norm_formatter(
                        str.template sub<Begin, Idx + 1 - Begin>()))>;
                return scan_norm<Begin, Idx + 1, cur_res>(str);
            } else {
                return Prev{};
            }
        } else {
            return Prev{};
        }
    }
    template <std::size_t Idx, char... Cs>
    static constexpr auto parse1(string<Cs...> str) {
        if constexpr (Idx != sizeof...(Cs)) {
            using __norm_result =
                decltype(scan_norm<Idx, Idx, failed_result>(str));
            using __formatter_result =
                decltype(scan_formatter0<Idx, 2, failed_result>(str));
            static_assert(!std::is_same_v<__norm_result, __formatter_result>,
                          "bad format string, there may be invalid specifiers");
            if constexpr (!std::is_same_v<__norm_result, failed_result>) {
                return std::tuple_cat(std::tuple<__norm_result>(),
                                      parse1<__norm_result::end>(str));
            } else {
                return std::tuple_cat(std::tuple<__formatter_result>(),
                                      parse1<__formatter_result::end>(str));
            }
        } else {
            return std::tuple{};
        }
    }
    template <std::size_t Type, std::size_t ArgPos, typename Formatter>
    struct final_section {
        constexpr static inline std::size_t type = Type;
        constexpr static inline std::size_t arg_pos = ArgPos;
        using formatter_type = Formatter;

        formatter_type formatter;
    };
    template <std::size_t Idx, std::size_t ArgPos, typename... Ts>
    static constexpr auto make_final_section(std::tuple<Ts...> tp) {
        if constexpr (Idx != sizeof...(Ts)) {
            if constexpr (std::get<Idx>(tp).type == 0) {
                return std::tuple_cat(
                    std::tuple<final_section<
                        0, ArgPos,
                        typename std::tuple_element_t<
                            Idx, std::tuple<Ts...>>::formatter_type>>{},
                    make_final_section<Idx + 1, ArgPos + 1>(tp));
            } else {
                return std::tuple_cat(
                    std::tuple<final_section<
                        1, ArgPos,
                        typename std::tuple_element_t<
                            Idx, std::tuple<Ts...>>::formatter_type>>{},
                    make_final_section<Idx + 1, ArgPos>(tp));
            }
        } else {
            return std::tuple{};
        }
    }

    template <char... Cs> constexpr static auto final_parse(string<Cs...> str) {
        return make_final_section<0, 0>(parse1<0>(str));
    }
    template <char... Cs>
    constexpr static auto parse_formatter(string<Cs...> str) {
        using parse_result =
            decltype(scan_formatter0<0, 2, failed_result>(str));
        static_assert(!std::is_same_v<parse_result, failed_result>);
        static_assert(parse_result::end == str.size);
        return typename parse_result::formatter_type{};
    }

    template <std::size_t Idx, typename... Ts>
    constexpr static bool call_path_check_max_size(std::tuple<Ts...> ts) {
        if constexpr (Idx != sizeof...(Ts)) {
            if constexpr (has_max_size<typename std::tuple_element_t<
                              Idx, std::tuple<Ts...>>::formatter_type>::value) {
                if constexpr (typename std::tuple_element_t<
                                  Idx, std::tuple<Ts...>>::formatter_type()
                                  .max_size() == 0) {
                    return false;
                } else {
                    return call_path_check_max_size<Idx + 1>(ts);
                }
            } else {
                return false;
            }
        } else {
            return true;
        }
    }
    template <std::size_t Idx, typename... Ts>
    static constexpr std::size_t call_path_max_size(std::tuple<Ts...> ts) {
        if constexpr (Idx != sizeof...(Ts)) {
            return typename std::tuple_element_t<
                       Idx, std::tuple<Ts...>>::formatter_type()
                       .max_size() +
                   call_path_max_size<Idx + 1>(ts);
        } else {
            return 0;
        }
    }

    template <std::size_t Idx, typename... Ts, typename... Rs>
    constexpr static auto call_path_get_tp_type(std::tuple<Ts...> ts,
                                                std::tuple<Rs...>& rs) {
        if constexpr (Idx != sizeof...(Ts)) {
            using current_section =
                std::tuple_element_t<Idx, std::tuple<Ts...>>;
            if constexpr (current_section::type == 0 &&
                          !std::is_same_v<
                              typename current_section::formatter_type,
                              typename FormatterSpec::template rebind<
                                  flags<>, specifier<'%'>>>) {
                return std::tuple_cat(
                    std::tuple<
                        decltype(formatter_context<Idx,
                                                   current_section::arg_pos>::
                                     invoke_calculate_size(
                                         typename current_section::
                                             formatter_type(),
                                         std::get<current_section::arg_pos>(rs),
                                         rs))>(),
                    call_path_get_tp_type<Idx + 1>(ts, rs));
            } else {
                return std::tuple_cat(
                    std::tuple<
                        decltype(typename current_section::formatter_type()
                                     .calculate_size())>(),
                    call_path_get_tp_type<Idx + 1>(ts, rs));
            }
        } else {
            return std::tuple{};
        }
    }
    template <std::size_t Idx, typename R, typename... Ts>
    constexpr static auto call_path_get_tp_type2(std::tuple<Ts...> ts) {
        if constexpr (Idx != sizeof...(Ts)) {
            using current_section =
                std::tuple_element_t<Idx, std::tuple<Ts...>>;
            if constexpr (current_section::type == 0 &&
                          !std::is_same_v<
                              typename current_section::formatter_type,
                              typename FormatterSpec::template rebind<
                                  flags<>, specifier<'%'>>>) {
                return std::tuple_cat(
                    std::tuple<
                        decltype(typename current_section::formatter_type()
                                     .calculate_size(std::declval<R>()))>(),
                    call_path_get_tp_type2<Idx + 1, R>(ts));
            } else {
                return std::tuple_cat(
                    std::tuple<
                        decltype(typename current_section::formatter_type()
                                     .calculate_size())>(),
                    call_path_get_tp_type2<Idx + 1, R>(ts));
            }
        } else {
            return std::tuple{};
        }
    }

    template <std::size_t Idx, typename... Ts, typename... Rs, typename... Ns>
    constexpr std::size_t static call_path_size1(std::tuple<Ts...>& ts,
                                                 std::tuple<Rs...>& rs,
                                                 std::tuple<Ns...>& ns) {
        if constexpr (Idx != sizeof...(Ts)) {
            using current_section =
                std::tuple_element_t<Idx, std::tuple<Ts...>>;
            if constexpr (current_section::type == 0 &&
                          !std::is_same_v<
                              typename current_section::formatter_type,
                              typename FormatterSpec::template rebind<
                                  flags<>, specifier<'%'>>>) {
                auto _s = formatter_context<Idx, current_section::arg_pos>::
                    invoke_calculate_size(
                        std::get<Idx>(ts).formatter,
                        std::get<current_section::arg_pos>(rs), rs);
                std::get<Idx>(ns) = _s;
                if constexpr (std::is_same_v<decltype(_s), std::size_t>) {
                    return _s + call_path_size1<Idx + 1>(ts, rs, ns);
                } else {
                    return _s.full_size + call_path_size1<Idx + 1>(ts, rs, ns);
                }
            } else {
                return std::get<Idx>(ns).full_size +
                       call_path_size1<Idx + 1>(ts, rs, ns);
            }
        } else {
            return 0;
        }
    }

    template <std::size_t Idx, typename... Ts, typename R, typename Ns>
    constexpr std::size_t static call_path_size2(std::tuple<Ts...>& ts, R& r,
                                                 Ns& ns) {
        if constexpr (Idx != sizeof...(Ts)) {
            using current_section =
                std::tuple_element_t<Idx, std::tuple<Ts...>>;
            if constexpr (current_section::type == 0 &&
                          !std::is_same_v<
                              typename current_section::formatter_type,
                              typename FormatterSpec::template rebind<
                                  flags<>, specifier<'%'>>>) {
                auto _s = std::get<Idx>(ts).formatter.calculate_size(r);
                std::get<Idx>(ns) = _s;
                if constexpr (std::is_same_v<decltype(_s), std::size_t>) {
                    return _s + call_path_size2<Idx + 1>(ts, r, ns);
                } else {
                    return _s.full_size + call_path_size2<Idx + 1>(ts, r, ns);
                }
            } else {
                return std::get<Idx>(ns).full_size +
                       call_path_size2<Idx + 1>(ts, r, ns);
            }
        } else {
            return 0;
        }
    }

    template <std::size_t Idx, typename... Ts, typename... Rs, typename... Ns,
              typename RandomAccessIterator>
    static constexpr void
    call_path_format1(RandomAccessIterator iter, std::tuple<Ts...>& ts,
                      std::tuple<Rs...>& rs, std::tuple<Ns...>& ns) {
        if constexpr (Idx != sizeof...(Ts)) {
            using current_section =
                std::tuple_element_t<Idx, std::tuple<Ts...>>;
            if constexpr (current_section::type == 0 &&
                          !std::is_same_v<
                              typename current_section::formatter_type,
                              typename FormatterSpec::template rebind<
                                  flags<>, specifier<'%'>>>) {
                formatter_context<Idx, current_section::arg_pos>::invoke_format(
                    std::get<Idx>(ts).formatter,
                    std::get<current_section::arg_pos>(rs), rs, iter,
                    std::get<Idx>(ns));
            } else {
                std::get<Idx>(ts).formatter.format(iter, std::get<Idx>(ns));
            }
            if constexpr (std::is_same_v<
                              std::tuple_element_t<Idx, std::tuple<Ns...>>,
                              std::size_t>) {
                call_path_format1<Idx + 1>(iter + std::get<Idx>(ns), ts, rs,
                                           ns);
            } else {
                call_path_format1<Idx + 1>(iter + std::get<Idx>(ns).full_size,
                                           ts, rs, ns);
            }
        }
    }

    template <std::size_t Idx, typename... Ts, typename R, typename Ns,
              typename RandomAccessIterator>
    static constexpr void call_path_format2(RandomAccessIterator iter,
                                            std::tuple<Ts...>& ts, R& r,
                                            Ns& ns) {
        if constexpr (Idx != sizeof...(Ts)) {
            using current_section =
                std::tuple_element_t<Idx, std::tuple<Ts...>>;
            if constexpr (current_section::type == 0 &&
                          !std::is_same_v<
                              typename current_section::formatter_type,
                              typename FormatterSpec::template rebind<
                                  flags<>, specifier<'%'>>>) {
                std::get<Idx>(ts).formatter.format(iter, r, std::get<Idx>(ns));
            } else {
                std::get<Idx>(ts).formatter.format(iter, std::get<Idx>(ns));
            }
            if constexpr (std::is_same_v<
                              std::decay_t<decltype(std::get<Idx>(ns))>,
                              std::size_t>) {
                call_path_format2<Idx + 1>(iter + std::get<Idx>(ns), ts, r, ns);
            } else {
                call_path_format2<Idx + 1>(iter + std::get<Idx>(ns).full_size,
                                           ts, r, ns);
            }
        }
    }

    template <char... Cs, typename... Rs>
    constexpr static std::size_t formatted_size(string<Cs...> pattern,
                                                Rs&&... rs) {
        using sections = decltype(make_final_section<0, 0>(parse1<0>(pattern)));
        if constexpr (std::tuple_size_v<sections> == 0) {
            return 0;
        } else {
            sections sts;
            std::tuple<Rs&&...> rts(std::forward<Rs>(rs)...);
            decltype(call_path_get_tp_type<0>(sts, rts)) nts;
            return call_path_size1<0>(sts, rts, nts);
        }
    }

    template <char... Cs, typename... Rs>
    static std::string format(string<Cs...> pattern, Rs&&... rs) {
        using sections = decltype(make_final_section<0, 0>(parse1<0>(pattern)));
        if constexpr (std::tuple_size_v<sections> == 0) {
            return {};
        } else {
            std::tuple<Rs&&...> rts(std::forward<Rs&&>(rs)...);
            sections sts;
            decltype(call_path_get_tp_type<0>(sts, rts)) nts;
            if constexpr (has_max_size<sections>::value) {
                char buffer[call_path_max_size<0>(sts)] = {};
                std::size_t requested_size = call_path_size1<0>(sts, rts, nts);
                call_path_format1<0>(buffer, sts, rts, nts);
                return buffer;
            } else {
                std::size_t requested_size = call_path_size1<0>(sts, rts, nts);
                std::string output;
                output.resize(requested_size);
                call_path_format1<0>(output.data(), sts, rts, nts);
                return std::move(output);
            }
        }
    }

    template <typename RandomAccessIterator, char... Cs, typename... Rs>
    static void format_to(RandomAccessIterator iter, string<Cs...> pattern,
                          Rs&&... rs) {
        using sections = decltype(make_final_section<0, 0>(parse1<0>(pattern)));
        if constexpr (std::tuple_size_v<sections> != 0) {
            std::tuple<Rs&&...> rts(std::forward<Rs&&>(rs)...);
            sections sts;
            decltype(call_path_get_tp_type<0>(sts, rts)) nts;
            call_path_size1<0>(sts, rts, nts);
            call_path_format1<0>(iter, sts, rts, nts);
        }
    }

    // use with crtp
    template <typename Self, typename Context, char... SubFormatString>
    struct subformatter_helper {
        using helper_type = subformatter_helper;
        using context_type = Context;
        using sections_type =
            decltype(final_parse(string<SubFormatString...>{}));
        sections_type stp;
        decltype(call_path_get_tp_type2<0, context_type>(stp)) ntp;

        constexpr std::size_t max_size() noexcept(true) {
            return max_size_impl<0>();
        }
        constexpr std::size_t
        calculate_size(const context_type& t) noexcept(true) {
            return call_path_size2<0>(stp, t, ntp);
        }
        template <typename RandomAccessIterator>
        constexpr void format(RandomAccessIterator iter, const context_type& t,
                              std::size_t) noexcept(true) {
            return call_path_format2<0>(iter, stp, t, ntp);
        }

      private:
        template <std::size_t Idx> constexpr std::size_t max_size_impl() {
            if constexpr (std::tuple_size_v<sections_type> == Idx + 1) {
                return std::get<Idx>(stp).formatter.max_size();
            } else {
                constexpr std::size_t follow = max_size_impl<Idx + 1>();
                if constexpr (follow == 0) {
                    return 0;
                } else {
                    return follow + std::get<Idx>(stp).formatter.max_size();
                }
            }
        }
    };
};
}  // namespace detail

template <char... Cs, typename... Rs>
std::string format(string<Cs...> str, Rs&&... rs) {
    return detail::core2<>::format(str, std::forward<Rs>(rs)...);
}
template <typename RandomAccessIterator, char... Cs, typename... Rs>
void format_to(RandomAccessIterator iter, string<Cs...> str, Rs&&... rs) {
    return detail::core2<>::format_to(iter, str, std::forward<Rs>(rs)...);
}
template <char... Cs, typename... Rs>
constexpr std::size_t formatted_size(string<Cs...> str, Rs&&... rs) {
    return detail::core2<>::formatted_size(str, std::forward<Rs>(rs)...);
}

namespace literals {
template <typename T, T... Cs>
constexpr string<Cs...> operator""_str() noexcept(true) {
    // it's amazing that even literal operator can be a template function.
    // global namespace is polluted, still. and not possible for standard C++17.
    return {};
}
}  // namespace literals
}  // namespace chx::log
