#pragma once

#include <cstddef>

namespace lumina::meta {
    struct ECSTypeContext {};
    struct EventsTypeContext {};

    template <typename T>
    inline constexpr bool IsTypeContext = false;

    template <>
    inline constexpr bool IsTypeContext<ECSTypeContext> = true;

    template <>
    inline constexpr bool IsTypeContext<EventsTypeContext> = true;

    template <typename T>
    concept TypeContext = IsTypeContext<T>;

    template <TypeContext Ctx>
    struct TypeIndexer final {
        using ContextType = Ctx;

        template <typename T>
        [[nodiscard]] static std::size_t index() {
            static std::size_t n = Counter++;
            return n;
        }

        inline static std::size_t Counter = 0;
    };
}
