#pragma once

#include <lumina/core/application.hpp>

#include "sparse_set.hpp"
#include "entity.hpp"
#include "view.hpp"

namespace lumina::ecs {
    class Registry final {
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

        template <Component T>
        T& emplace(const Entity& target);

        template <Component T>
        T& emplace(const Entity& target, const T& value);

        template <Component T>
        T& emplace(const Entity& target, T&& value);

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

        template <Component T>
        [[nodiscard]] SparseSet<T> sparseSet() {
            std::size_t index = acquire<T>();

            return {indexTables_[index], memoryPools_[index]};
        }

    private:
        std::vector<MemoryPool> memoryPools_;
        std::vector<IndexTable> indexTables_;
        std::vector<std::size_t> typeSizes_;
        std::vector<std::size_t> versions_;
        std::vector<std::size_t> freeList_;
        std::vector<bool> statuses_;

        bool validation_;

        template <Component T>
        [[nodiscard]] std::size_t acquire();

        template <Component T>
        [[nodiscard]] bool acquirable() const;
    };
}
