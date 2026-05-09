#pragma once

#include <lumina/meta/allocation.hpp>
#include <lumina/meta/index_table.hpp>
#include <lumina/meta/pod_type.hpp>

#include "entity.hpp"

namespace lumina::ecs {
    class SetFunctions final {
        using Allocation = meta::Allocation;
        using IndexType = Entity::ValueType;
        using IndexTable = meta::IndexTable<IndexType>;

    public:
        SetFunctions() = delete;

        template <meta::PODType T>
        [[nodiscard]] static T& get(Allocation& allocation, const IndexTable& table, IndexType sparse);

        template <meta::PODType T>
        [[nodiscard]] static const T& get(const Allocation& allocation, const IndexTable& table, IndexType sparse);

        template <meta::PODType T, typename... Args>
        static T& insert(Allocation& allocation, IndexTable& table, IndexType sparse, Args&&... args);
        static void remove(Allocation& allocation, IndexTable& table, IndexType sparse);
    };
}
