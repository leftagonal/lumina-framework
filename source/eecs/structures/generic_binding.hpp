#pragma once

#include <eecs/traits/component.hpp>

#include <eecs/structures/allocation.hpp>
#include <eecs/structures/index_table.hpp>

namespace eecs {
    class generic_binding final {
    public:
        generic_binding() = default;
        ~generic_binding() = default;

        generic_binding(const generic_binding&) = default;
        generic_binding(generic_binding&&) noexcept = default;

        generic_binding& operator=(const generic_binding&) = default;
        generic_binding& operator=(generic_binding&&) noexcept = default;

        [[nodiscard]] bool operator==(const generic_binding& other) const {
            return remove_ == other.remove_;
        }

        [[nodiscard]] bool operator!=(const generic_binding& other) const {
            return !(*this == other);
        }

        explicit operator bool() const {
            return bound();
        }

        template <component T>
        void bind() {
            remove_ = remove<T>;
        }

        void unbind() {
            remove_ = nullptr;
        }

        [[nodiscard]] bool bound() const {
            return remove_ != nullptr;
        }

        void remove(allocation& allocation, index_table& index_table, std::uint32_t sparse) {
            remove_(allocation, index_table, sparse);
        }

    private:
        void (*remove_)(allocation&, index_table&, std::uint32_t) = nullptr;

        template <component T>
        static void remove(allocation& allocation, index_table& index_table, index_table::type sparse) {
            index_table::type dense = index_table[sparse];

            std::byte* target = &allocation[dense * sizeof(T)];
            std::byte* end = &allocation[allocation.size() - sizeof(T)];

            std::memmove(target, end, sizeof(T));

            index_table.remove(sparse);
            allocation.shrink(sizeof(T));
        }
    };
}
