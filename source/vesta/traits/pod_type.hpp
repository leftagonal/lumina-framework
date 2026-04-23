#pragma once

#include <type_traits>

namespace vesta::internal {
    template <typename T>
    concept pod_type = std::is_trivially_destructible_v<T> &&
                       std::is_trivially_move_constructible_v<T> &&
                       std::is_trivially_copy_constructible_v<T> &&
                       std::is_standard_layout_v<T>;

    template <typename T>
    inline constexpr bool is_pod_type_v = pod_type<T>;
}