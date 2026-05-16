#pragma once

#include <type_traits>

namespace lumina::meta {
    template <typename T>
    requires(std::is_arithmetic_v<T>)
    struct Extent2D {
        using Type = T;

        T width;
        T height;
    };
}
