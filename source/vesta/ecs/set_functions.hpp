#pragma once

#include <vesta/traits/pod_type.hpp>

#include <vesta/ecs/entity.hpp>

#include <vesta/structures/allocation.hpp>
#include <vesta/structures/index_table.hpp>

namespace vesta::internal {
    class set_functions final {
        using index_type = entity::value_type;
        using index_table = index_table<index_type>;

    public:
        set_functions() = delete;

        template <pod_type T>
        [[nodiscard]] static T& get(allocation& allocation, const index_table& table, index_type sparse) {
            index_type dense = table[sparse];
            std::byte* target = &allocation[dense];

            return *reinterpret_cast<T*>(target);
        }

        template <pod_type T>
        [[nodiscard]] static const T& get(
            const allocation& allocation,
            const index_table& table,
            index_type sparse) {
            index_type dense = table[sparse];
            const std::byte* target = &allocation[dense];

            return *reinterpret_cast<const T*>(target);
        }

        template <pod_type T, typename... Args>
        static T& insert(allocation& allocation, index_table& table, index_type sparse, Args&&... args) {
            std::size_t index = allocation.count();

            table.insert(sparse);
            allocation.extend(1);

            std::byte* element = &allocation[index];

            ::new (element) T(std::forward<Args>(args)...);

            return *reinterpret_cast<T*>(element);
        }

        static void remove(allocation& allocation, index_table& table, index_type sparse) {

            std::byte* target = &allocation[table[sparse]];
            std::byte* end = &allocation[allocation.count() - 1];

            if (target != end) {
                table.move_to_end(sparse);
                std::memmove(target, end, allocation.stride());
            }

            table.pop();
            allocation.shrink(1);
        }
    };
}
