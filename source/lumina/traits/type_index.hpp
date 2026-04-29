#pragma once

#include <lumina/traits/empty_type.hpp>

namespace lumina::traits {
    using TypeIndex = std::size_t;

    template <EmptyType T>
    inline constexpr bool IsTypeContext = false;

    struct ECSTypeContext {};

    template <>
    inline constexpr bool IsTypeContext<ECSTypeContext> = true;

    template <EmptyType>
    [[nodiscard]] inline TypeIndex incrementContext() {
        static TypeIndex value = 0xFFFFFFFFFFFFFFFF;

        return ++value;
    }

    template <EmptyType Context, typename>
    requires(IsTypeContext<Context>)
    [[nodiscard]] TypeIndex typeIndex() {
        static const TypeIndex index = incrementContext<Context>();

        return index;
    }
}
