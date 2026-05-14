#pragma once

#include <type_traits>

namespace lumina::meta {
    template <typename T>
    concept PODType = std::is_trivially_default_constructible_v<T> &&
                      std::is_trivially_destructible_v<T> &&
                      std::is_trivially_move_constructible_v<T> &&
                      std::is_trivially_copy_constructible_v<T> &&
                      std::is_standard_layout_v<T>;

    template <typename T>
    inline constexpr bool IsPODType = PODType<T>;
}
