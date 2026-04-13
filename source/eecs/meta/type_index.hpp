#pragma once

#include <cstddef>

namespace eecs {
    namespace internal {
        [[nodiscard]] inline std::size_t increment() {
            static std::size_t value = 0xFFFFFFFFFFFFFFFF;

            return ++value;
        }
    }

    template <typename T>
    [[nodiscard]] std::size_t type_index() {
        static std::size_t index = internal::increment();

        return index;
    }
}
