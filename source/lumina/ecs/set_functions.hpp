#pragma once

#include <lumina/traits/pod_type.hpp>

#include <lumina/ecs/entity.hpp>

#include <lumina/structs/allocation.hpp>
#include <lumina/structs/index_table.hpp>

namespace lumina::ecs {
    class set_functions final {
        using Allocation = structs::Allocation;
        using IndexType = Entity::ValueType;
        using IndexTable = structs::IndexTable<IndexType>;

    public:
        set_functions() = delete;

        template <traits::PODType T>
        [[nodiscard]] static T& get(Allocation& Allocation, const IndexTable& table, IndexType sparse) {
            IndexType dense = table[sparse];
            std::byte* target = &Allocation[dense];

            return *reinterpret_cast<T*>(target);
        }

        template <traits::PODType T>
        [[nodiscard]] static const T& get(
            const Allocation& Allocation,
            const IndexTable& table,
            IndexType sparse) {
            IndexType dense = table[sparse];
            const std::byte* target = &Allocation[dense];

            return *reinterpret_cast<const T*>(target);
        }

        template <traits::PODType T, typename... Args>
        static T& insert(Allocation& Allocation, IndexTable& table, IndexType sparse, Args&&... args) {
            std::size_t index = Allocation.count();

            table.insert(sparse);
            Allocation.extend(1);

            std::byte* element = &Allocation[index];

            ::new (element) T(std::forward<Args>(args)...);

            return *reinterpret_cast<T*>(element);
        }

        static void remove(Allocation& Allocation, IndexTable& table, IndexType sparse) {

            std::byte* target = &Allocation[table[sparse]];
            std::byte* end = &Allocation[Allocation.count() - 1];

            if (target != end) {
                table.moveToEnd(sparse);
                std::memmove(target, end, Allocation.stride());
            }

            table.pop();
            Allocation.shrink(1);
        }
    };
}
