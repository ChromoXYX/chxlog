#pragma once

#include "../carrier.hpp"

namespace chx::log::detail {
template <typename T, std::size_t... Idx>
constexpr auto make_string(T t, std::integer_sequence<std::size_t, Idx...>) {
    return string<t()[Idx]...>{};
}

#define CHXLOG_STR(x)                                                          \
    ::chx::log::detail::make_string(                                           \
        []() { return x; },                                                    \
        std::make_integer_sequence<std::size_t, sizeof(x) - 1>{})
}  // namespace chx::log::detail
