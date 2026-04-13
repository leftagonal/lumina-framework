#pragma once

#include <eecs/structures/allocation.hpp>
#include <eecs/structures/component_store.hpp>
#include <eecs/structures/entity.hpp>
#include <eecs/structures/generic_binding.hpp>
#include <eecs/structures/index_table.hpp>

#include <eecs/meta/type_index.hpp>

namespace eecs {
    class registry final {
    public:
        registry() = default;
        ~registry() = default;

        registry(const registry&) = default;
        registry(registry&&) noexcept = default;

        registry& operator=(const registry&) = default;
        registry& operator=(registry&&) noexcept = default;

        [[nodiscard]] entity create() {
            index_table::type id;
            index_table::type version;

            if (!free_list_.empty()) {
                id = free_list_.back();
                version = versions_[id];

                free_list_.pop_back();
                statuses_[id] = false;

            } else {
                id = static_cast<index_table::type>(versions_.size());
                version = versions_.emplace_back(0);

                statuses_.emplace_back(true);
            }

            return entity{id, version};
        }

        [[nodiscard]] bool contains(const entity& target) const {
            if (!target) {
                return false;
            }

            index_table::type id = target.id();
            index_table::type version = target.version();

            return versions_.size() > id && versions_[id] == version && statuses_[id];
        }

        void destroy(const entity& target) {
            if (!contains(target)) {
                return;
            }

            index_table::type id = target.id();

            remove_all(target);

            versions_[id]++;
            free_list_.emplace_back(id);
            statuses_[id] = false;
        }

        void clear() {
            allocations_.clear();
            index_tables_.clear();
            generic_bindings_.clear();

            versions_.clear();
            free_list_.clear();
            statuses_.clear();
        }

        template <component T, typename... Args>
        T& emplace(const entity& target, Args&&... args) {
            std::size_t index = acquire<T>();

            allocation& allocation = allocations_[index];
            index_table& table = index_tables_[index];

            return component_store<T>::insert(allocation, table, target.id(), std::forward<Args>(args)...);
        }

        template <component T>
        [[nodiscard]] bool has(const entity& target) const {
            if (!acquirable<T>()) {
                return false;
            }

            std::size_t index = type_index<T>();

            const index_table& table = index_tables_[index];

            return table.contains(target.id());
        }

        template <component T>
        [[nodiscard]] T& get(const entity& target) {
            std::size_t index = type_index<T>();

            allocation& allocation = allocations_[index];
            index_table& table = index_tables_[index];

            return component_store<T>::get(allocation, table, target.id());
        }

        template <component T>
        [[nodiscard]] const T& get(const entity& target) const {
            std::size_t index = type_index<T>();

            const allocation& allocation = allocations_[index];
            const index_table& table = index_tables_[index];

            return component_store<T>::get(allocation, table, target.id());
        }

        template <component T>
        void remove(const entity& target) {
            std::size_t index = type_index<T>();

            allocation& allocation = allocations_[index];
            index_table& table = index_tables_[index];
            generic_binding& binding = generic_bindings_[index];

            binding.remove(allocation, table, target.id());
        }

        void remove_all(const entity& target) {
            for (std::size_t i = 0; i < generic_bindings_.size(); i++) {
                index_table& table = index_tables_[i];

                if (table.contains(target.id())) {
                    allocation& allocation = allocations_[i];
                    generic_binding& binding = generic_bindings_[i];

                    binding.remove(allocation, table, target.id());
                }
            }
        }

    private:
        std::vector<allocation> allocations_;
        std::vector<index_table> index_tables_;
        std::vector<generic_binding> generic_bindings_;

        std::vector<index_table::type> versions_;
        std::vector<index_table::type> free_list_;
        std::vector<bool> statuses_;

        template <component T>
        [[nodiscard]] std::size_t acquire() {
            std::size_t index = type_index<T>();

            if (index >= allocations_.size()) {
                allocations_.resize(index + 1);
                index_tables_.resize(index + 1);
                generic_bindings_.resize(index + 1);
            }

            generic_binding& binding = generic_bindings_[index];

            if (!binding) {
                binding.bind<T>();
            }

            return index;
        }

        template <component T>
        [[nodiscard]] bool acquirable() const {
            std::size_t index = type_index<T>();

            return index < allocations_.size();
        }
    };
}
