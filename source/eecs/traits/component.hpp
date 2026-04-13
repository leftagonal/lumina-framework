#pragma once

#include <type_traits>

namespace eecs {
    template <typename T>
    concept component = std::is_trivial_v<T> && std::is_standard_layout_v<T>;

    template <typename T>
    inline constexpr bool is_component_v = component<T>;
}
