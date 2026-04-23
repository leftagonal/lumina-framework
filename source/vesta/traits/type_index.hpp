#pragma once

#include <vesta/traits/tag_type.hpp>

#include <cstddef>

namespace vesta::internal {
    template <tag_type T>
    inline constexpr bool is_type_context_v = false;

    namespace internal {
        template <typename>
        [[nodiscard]] inline std::size_t increment() {
            static std::size_t value = 0xFFFFFFFFFFFFFFFF;

            return ++value;
        }
    }

    template <typename T, typename U>
    requires(is_type_context_v<T>)
    [[nodiscard]] std::size_t type_index() {
        static std::size_t index = internal::increment<T>();

        return index;
    }
}
