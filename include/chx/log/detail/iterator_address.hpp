#pragma once

#include <type_traits>

namespace chx::log::detail {
template <typename RandomAccessIterator>
constexpr auto* iter_addr(RandomAccessIterator iter) noexcept(true) {
    if constexpr (std::is_pointer_v<RandomAccessIterator>) {
        return static_cast<char*>(static_cast<void*>(iter));
    } else {
        return static_cast<char*>(static_cast<void*>(&*iter));
    }
}
}  // namespace chx::log::detail
