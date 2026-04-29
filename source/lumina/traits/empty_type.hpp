#pragma once

#include <type_traits>

namespace lumina::traits {
    template <typename T>
    concept EmptyType = std::is_empty_v<T> &&
                        std::is_standard_layout_v<T>;

    template <typename T>
    inline constexpr bool IsEmptyType = EmptyType<T>;
}