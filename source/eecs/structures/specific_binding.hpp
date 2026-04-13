#pragma once

#include <eecs/traits/component.hpp>

#include <eecs/structures/allocation.hpp>
#include <eecs/structures/index_table.hpp>

namespace eecs {
    template <component T>
    class specific_binding final {
    public:
        using type = T;

        specific_binding(allocation& allocation, index_table& index_table)
            : allocation_(&allocation), index_table_(&index_table) {
        }

        ~specific_binding() = default;

        specific_binding(const specific_binding&) = default;
        specific_binding(specific_binding&&) noexcept = default;

        specific_binding& operator=(const specific_binding&) = default;
        specific_binding& operator=(specific_binding&&) noexcept = default;

        [[nodiscard]] type& operator[](index_table::type sparse) {
            index_table::type dense = (*index_table_)[sparse];
            std::byte* target = &(*allocation_)[dense * sizeof(type)];

            return *reinterpret_cast<type*>(target);
        }

        [[nodiscard]] const type& operator[](index_table::type sparse) const {
            index_table::type dense = (*index_table_)[sparse];
            const std::byte* target = &(*allocation_)[dense * sizeof(type)];

            return *reinterpret_cast<const type*>(target);
        }

        template <typename... Args>
        type& insert(index_table::type sparse, Args&&... args) {
            std::size_t index = allocation_->size();

            index_table_->insert(sparse);
            allocation_->extend(sizeof(type));

            std::byte* element = &(*allocation_)[index];

            ::new (element) type(std::forward<Args>(args)...);

            return *reinterpret_cast<type*>(element);
        }

        void remove(index_table::type sparse) {
            index_table::type dense = (*index_table_)[sparse];

            std::byte* target = &(*allocation_)[dense * sizeof(T)];
            std::byte* end = &(*allocation_)[allocation_->size() - sizeof(T)];

            std::memmove(target, end, sizeof(T));

            index_table_->remove(sparse);
            allocation_->shrink(sizeof(T));
        }

    private:
        allocation* allocation_ = nullptr;
        index_table* index_table_ = nullptr;
    };
}
