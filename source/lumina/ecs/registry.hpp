#pragma once

#include <lumina/meta/allocation.hpp>
#include <lumina/meta/index_table.hpp>
#include <lumina/meta/pod_type.hpp>

#include "entity.hpp"
#include "view.hpp"

namespace lumina::ecs {
    class Registry final {
        using IndexType = Entity::ValueType;
        using IndexTable = meta::IndexTable<IndexType>;
        using Allocation = meta::Allocation;

    public:
        Registry() = default;
        ~Registry() = default;

        Registry(const Registry&) = default;
        Registry(Registry&&) noexcept = default;

        Registry& operator=(const Registry&) = default;
        Registry& operator=(Registry&&) noexcept = default;

        [[nodiscard]] Entity create();
        [[nodiscard]] bool contains(const Entity& target) const;

        void destroy(const Entity& target);
        void clear();

        template <meta::PODType T, typename... Args>
        T& emplace(const Entity& target, Args&&... args);

        template <meta::PODType T>
        [[nodiscard]] bool has(const Entity& target) const;

        template <meta::PODType T>
        [[nodiscard]] T& get(const Entity& target);

        template <meta::PODType T>
        [[nodiscard]] const T& get(const Entity& target) const;

        template <meta::PODType T>
        void remove(const Entity& target);
        void removeAll(const Entity& target);

        template <meta::PODType... Ts>
        [[nodiscard]] View<Ts...> view();

        template <meta::PODType... Ts>
        [[nodiscard]] ConstView<Ts...> view() const;

    private:
        std::vector<Allocation> allocations_;
        std::vector<IndexTable> indexTables_;

        std::vector<IndexType> versions_;
        std::vector<IndexType> freeList_;
        std::vector<bool> statuses_;

        template <meta::PODType T>
        [[nodiscard]] static std::size_t typeIndex();

        template <meta::PODType T>
        [[nodiscard]] std::size_t acquire();

        template <meta::PODType T>
        [[nodiscard]] bool acquirable() const;
    };
}
