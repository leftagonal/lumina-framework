#pragma once

#include "empty_type.hpp"

namespace lumina::meta {
    using TypeIndex = std::size_t;

    template <EmptyType T>
    inline constexpr bool IsTypeContext = false;

    struct ECSTypeContext {};
    struct EventsTypeContext {};

    template <>
    inline constexpr bool IsTypeContext<ECSTypeContext> = true;

    template <>
    inline constexpr bool IsTypeContext<EventsTypeContext> = true;

    template <EmptyType>
    [[nodiscard]] inline TypeIndex incrementContext() {
        static TypeIndex value = 0xFFFFFFFFFFFFFFFFull;

        return ++value;
    }

    template <EmptyType Context, typename>
    requires(IsTypeContext<Context>)
    [[nodiscard]] TypeIndex typeIndex() {
        static const TypeIndex index = incrementContext<Context>();

        return index;
    }
}
