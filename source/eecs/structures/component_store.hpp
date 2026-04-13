#pragma once

#include <eecs/traits/component.hpp>

#include <eecs/structures/allocation.hpp>
#include <eecs/structures/index_table.hpp>

namespace eecs {
    template <component T>
    class component_store final {
    public:
        using type = T;

        component_store() = delete;

        [[nodiscard]] static type& get(allocation& allocation, const index_table& table, index_table::type sparse) {
            index_table::type dense = table[sparse];
            std::byte* target = &allocation[dense * sizeof(type)];

            return *reinterpret_cast<type*>(target);
        }

        [[nodiscard]] static const type& get(const allocation& allocation, const index_table& table, index_table::type sparse) {
            index_table::type dense = table[sparse];
            const std::byte* target = &allocation[dense * sizeof(type)];

            return *reinterpret_cast<const type*>(target);
        }

        template <typename... Args>
        static type& insert(allocation& allocation, index_table& table, index_table::type sparse, Args&&... args) {
            std::size_t index = allocation.size();

            table.insert(sparse);
            allocation.extend(sizeof(type));

            std::byte* element = &allocation[index];

            ::new (element) type(std::forward<Args>(args)...);

            return *reinterpret_cast<type*>(element);
        }

        static void remove(allocation& allocation, index_table& table, index_table::type sparse) {
            index_table::type dense = table[sparse];

            std::byte* target = &allocation[dense * sizeof(T)];
            std::byte* end = &allocation[allocation.size() - sizeof(T)];

            std::memmove(target, end, sizeof(T));

            table.remove(sparse);
            allocation.shrink(sizeof(T));
        }
    };
}
