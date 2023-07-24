#pragma once

#include "./format.hpp"

#include <algorithm>

namespace chx::log {
template <> struct formatter<flags<>, specifier<'s'>> {
    constexpr static std::size_t
    calculate_size(std::string_view sv) noexcept(true) {
        return sv.size();
    }
    template <typename RandomAccessIterator>
    constexpr static void format(RandomAccessIterator iter, std::string_view sv,
                                 std::size_t) noexcept(true) {
        std::copy_n(sv.begin(), sv.size(), iter);
    }
};

template <char... FixedSizeChar>
struct formatter<flags<sub_flags<>, sub_width<>, sub_precision<>,
                       sub_additional<FixedSizeChar...>>,
                 specifier<'s'>> {
    constexpr static inline std::size_t fixed_length =
        sub_additional<FixedSizeChar...>().to_number();
    constexpr static std::size_t max_size() noexcept(true) {
        return fixed_length;
    }
    constexpr static std::size_t
    calculate_size(std::string_view sv) noexcept(true) {
        return std::min(sv.size(), fixed_length);
    }
    template <typename RandomAccessIterator>
    constexpr static void format(RandomAccessIterator iter, std::string_view sv,
                                 std::size_t l) {
        std::copy_n(sv.begin(), l, iter);
    }
};
}  // namespace chx::log
