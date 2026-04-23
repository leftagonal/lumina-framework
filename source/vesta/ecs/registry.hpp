#pragma once

#include <vesta/structures/allocation.hpp>
#include <vesta/structures/index_table.hpp>

#include <vesta/ecs/entity.hpp>
#include <vesta/ecs/set_functions.hpp>

#include <vesta/traits/pod_type.hpp>
#include <vesta/traits/type_index.hpp>

namespace vesta {
    namespace internal {
        struct ecs_type_context_t {};

        template <>
        inline constexpr bool is_type_context_v<ecs_type_context_t> = true;
    }

    class registry final {
        using set_functions = internal::set_functions;
        using index_type = entity::value_type;
        using index_table = internal::index_table<index_type>;
        using allocation = internal::allocation;
        using context_type = internal::ecs_type_context_t;

    public:
        registry() = default;
        ~registry() = default;

        registry(const registry&) = default;
        registry(registry&&) noexcept = default;

        registry& operator=(const registry&) = default;
        registry& operator=(registry&&) noexcept = default;

        [[nodiscard]] entity create() {
            index_type id;
            index_type version;

            if (!free_list_.empty()) {
                id = free_list_.back();
                version = versions_[id];

                free_list_.pop_back();
                statuses_[id] = false;

            } else {
                id = static_cast<index_type>(versions_.size());
                version = versions_.emplace_back(0);

                statuses_.emplace_back(true);
            }

            return entity{id, version};
        }

        [[nodiscard]] bool contains(const entity& target) const {
            if (!target) {
                return false;
            }

            index_type id = target.id();
            index_type version = target.version();

            return versions_.size() > id && versions_[id] == version && statuses_[id];
        }

        void destroy(const entity& target) {
            if (!contains(target)) {
                return;
            }

            index_type id = target.id();

            remove_all(target);

            versions_[id]++;
            free_list_.emplace_back(id);
            statuses_[id] = false;
        }

        void clear() {
            allocations_.clear();
            index_tables_.clear();

            versions_.clear();
            free_list_.clear();
            statuses_.clear();
        }

        template <internal::pod_type T, typename... Args>
        T& emplace(const entity& target, Args&&... args) {
            std::size_t index = acquire<T>();

            allocation& allocation = allocations_[index];
            index_table& table = index_tables_[index];

            return set_functions::insert<T>(allocation, table, target.id(), std::forward<Args>(args)...);
        }

        template <internal::pod_type T>
        [[nodiscard]] bool has(const entity& target) const {
            if (!acquirable<T>()) {
                return false;
            }

            std::size_t index = type_index<T>();

            const index_table& table = index_tables_[index];

            return table.contains(target.id());
        }

        template <internal::pod_type T>
        [[nodiscard]] T& get(const entity& target) {
            std::size_t index = type_index<T>();

            allocation& allocation = allocations_[index];
            index_table& table = index_tables_[index];

            return set_functions::get<T>(allocation, table, target.id());
        }

        template <internal::pod_type T>
        [[nodiscard]] const T& get(const entity& target) const {
            std::size_t index = type_index<T>();

            const allocation& allocation = allocations_[index];
            const index_table& table = index_tables_[index];

            return set_functions::get<T>(allocation, table, target.id());
        }

        template <internal::pod_type T>
        void remove(const entity& target) {
            std::size_t index = type_index<T>();

            allocation& allocation = allocations_[index];
            index_table& table = index_tables_[index];

            set_functions::remove(allocation, table, target.id());
        }

        void remove_all(const entity& target) {
            for (std::size_t i = 0; i < allocations_.size(); ++i) {
                index_table& table = index_tables_[i];

                if (!table.contains(target.id())) {
                    return;
                }

                allocation& allocation = allocations_[i];
                index_type location = table[target.id()];

                const std::size_t stride = allocation.stride();
                const std::size_t size = allocation.size();

                std::size_t destination_offset = location * stride;
                std::size_t source_offset = size - stride;

                if (destination_offset < source_offset) {
                    std::byte* data = allocation.data();
                    std::byte* destination = data + destination_offset;
                    std::byte* source = data + source_offset;

                    std::memmove(destination, source, stride);
                }

                allocation.shrink(1);
                table.remove(target.id());
            }
        }

    private:
        std::vector<allocation> allocations_;
        std::vector<index_table> index_tables_;

        std::vector<index_type> versions_;
        std::vector<index_type> free_list_;
        std::vector<bool> statuses_;

        template <internal::pod_type T>
        [[nodiscard]] static std::size_t type_index() {
            return internal::type_index<context_type, T>();
        }

        template <internal::pod_type T>
        [[nodiscard]] std::size_t acquire() {
            std::size_t index = type_index<T>();

            if (index >= allocations_.size()) {
                allocations_.resize(std::max(4ul, index * 2));
                index_tables_.resize(std::max(4ul, index * 2));
            }

            if (!allocations_[index]) {
                allocations_[index].initialise(sizeof(T));
            }

            return index;
        }

        template <internal::pod_type T>
        [[nodiscard]] bool acquirable() const {
            std::size_t index = type_index<T>();

            return index < allocations_.size();
        }
    };
}
