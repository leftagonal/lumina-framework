#pragma once

#include <lumina/meta/allocation.hpp>
#include <lumina/meta/index_table.hpp>
#include <lumina/meta/pod_type.hpp>

#include <lumina/core/application.hpp>

#include "entity.hpp"
#include "view.hpp"

namespace lumina::ecs {
    class Registry final {
        using IndexType = Entity::ValueType;
        using IndexTable = meta::IndexTable<IndexType>;
        using Allocation = meta::Allocation;

    public:
        Registry(const core::ApplicationInfo& applicationInfo);
        ~Registry();

        Registry(const Registry&) = default;
        Registry(Registry&&) noexcept = default;

        Registry& operator=(const Registry&) = default;
        Registry& operator=(Registry&&) noexcept = default;

        [[nodiscard]] Entity create();
        [[nodiscard]] bool contains(const Entity& target) const;

        void destroy(const Entity& target);
        void clear();

        template <Component T, typename... Args>
        T& emplace(const Entity& target, Args&&... args);

        template <Component T>
        [[nodiscard]] bool has(const Entity& target) const;

        template <Component T>
        [[nodiscard]] T& get(const Entity& target);

        template <Component T>
        [[nodiscard]] const T& get(const Entity& target) const;

        template <Component T>
        void remove(const Entity& target);
        void removeAll(const Entity& target);

        template <Component... Ts>
        [[nodiscard]] View<Ts...> view();

        template <Component... Ts>
        [[nodiscard]] ConstView<Ts...> view() const;

    private:
        std::vector<Allocation> allocations_;
        std::vector<IndexTable> indexTables_;

        std::vector<IndexType> versions_;
        std::vector<IndexType> freeList_;
        std::vector<bool> statuses_;

        bool validation_;

        template <Component T>
        [[nodiscard]] std::size_t acquire();

        template <Component T>
        [[nodiscard]] bool acquirable() const;
    };
}
