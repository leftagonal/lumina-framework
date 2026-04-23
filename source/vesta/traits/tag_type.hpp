#pragma once

#include <type_traits>

namespace vesta::internal {
    template <typename T>
    concept tag_type = std::is_empty_v<T> &&
                       std::is_standard_layout_v<T>;

    template <typename T>
    inline constexpr bool is_tag_type_v = tag_type<T>;
}