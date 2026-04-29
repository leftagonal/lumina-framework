#pragma once

#include <type_traits>

namespace lumina::traits {
    template <typename T>
    concept PODType = std::is_trivially_destructible_v<T> &&
                      std::is_trivially_move_constructible_v<T> &&
                      std::is_trivially_copy_constructible_v<T> &&
                      std::is_standard_layout_v<T>;

    template <typename T>
    inline constexpr bool IsPODType = PODType<T>;
}