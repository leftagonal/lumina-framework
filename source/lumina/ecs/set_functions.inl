#pragma once

#include "set_functions.hpp"

namespace lumina::ecs {
    template <Component T>
    T& SetFunctions::get(Allocation& allocation, const IndexTable& table, IndexType sparse) {
        auto dense = table[sparse];
        auto* target = &allocation[dense];

        return *reinterpret_cast<T*>(target);
    }

    template <Component T>
    const T& SetFunctions::get(const Allocation& allocation, const IndexTable& table, IndexType sparse) {
        auto dense = table[sparse];
        auto* target = &allocation[dense];

        return *reinterpret_cast<const T*>(target);
    }

    template <Component T, typename... Args>
    T& SetFunctions::insert(Allocation& allocation, IndexTable& table, IndexType sparse, Args&&... args) {
        auto index = allocation.count();

        table.insert(sparse);
        allocation.extend(1);

        auto* element = &allocation[index];

        ::new (element) T(std::forward<Args>(args)...);

        return *reinterpret_cast<T*>(element);
    }

    void SetFunctions::remove(Allocation& allocation, IndexTable& table, IndexType sparse) {
        auto* target = &allocation[table[sparse]];
        auto* end = &allocation[allocation.count() - 1];

        if (target != end) {
            table.moveToEnd(sparse);

            std::memmove(target, end, allocation.stride());
        }

        table.pop();
        allocation.shrink(1);
    }
}
