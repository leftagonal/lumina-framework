#pragma once

#include <lumina/structs/allocation.hpp>
#include <lumina/structs/index_table.hpp>

#include <lumina/ecs/entity.hpp>
#include <lumina/ecs/set_functions.hpp>
#include <lumina/ecs/view.hpp>

#include <lumina/traits/pod_type.hpp>
#include <lumina/traits/type_index.hpp>

namespace lumina::ecs {
    class Registry final {
        using IndexType = Entity::ValueType;
        using IndexTable = structs::IndexTable<IndexType>;
        using Allocation = structs::Allocation;

    public:
        Registry() = default;
        ~Registry() = default;

        Registry(const Registry&) = default;
        Registry(Registry&&) noexcept = default;

        Registry& operator=(const Registry&) = default;
        Registry& operator=(Registry&&) noexcept = default;

        [[nodiscard]] Entity create() {
            IndexType id;
            IndexType version;

            if (!freeList_.empty()) {
                id = freeList_.back();
                version = versions_[id];

                freeList_.pop_back();
                statuses_[id] = false;

            } else {
                id = static_cast<IndexType>(versions_.size());
                version = versions_.emplace_back(0);

                statuses_.emplace_back(true);
            }

            return Entity{id, version};
        }

        [[nodiscard]] bool contains(const Entity& target) const {
            if (!target) {
                return false;
            }

            IndexType id = target.id();
            IndexType version = target.version();

            return versions_.size() > id && versions_[id] == version && statuses_[id];
        }

        void destroy(const Entity& target) {
            if (!contains(target)) {
                return;
            }

            IndexType id = target.id();

            removeAll(target);

            versions_[id]++;
            freeList_.emplace_back(id);
            statuses_[id] = false;
        }

        void clear() {
            allocations_.clear();
            index_tables_.clear();

            versions_.clear();
            freeList_.clear();
            statuses_.clear();
        }

        template <traits::PODType T, typename... Args>
        T& emplace(const Entity& target, Args&&... args) {
            std::size_t index = acquire<T>();

            Allocation& Allocation = allocations_[index];
            IndexTable& table = index_tables_[index];

            return set_functions::insert<T>(Allocation, table, target.id(), std::forward<Args>(args)...);
        }

        template <traits::PODType T>
        [[nodiscard]] bool has(const Entity& target) const {
            if (!acquirable<T>()) {
                return false;
            }

            std::size_t index = typeIndex<T>();

            const IndexTable& table = index_tables_[index];

            return table.contains(target.id());
        }

        template <traits::PODType T>
        [[nodiscard]] T& get(const Entity& target) {
            std::size_t index = typeIndex<T>();

            Allocation& Allocation = allocations_[index];
            IndexTable& table = index_tables_[index];

            return set_functions::get<T>(Allocation, table, target.id());
        }

        template <traits::PODType T>
        [[nodiscard]] const T& get(const Entity& target) const {
            std::size_t index = typeIndex<T>();

            const Allocation& Allocation = allocations_[index];
            const IndexTable& table = index_tables_[index];

            return set_functions::get<T>(Allocation, table, target.id());
        }

        template <traits::PODType T>
        void remove(const Entity& target) {
            std::size_t index = typeIndex<T>();

            Allocation& Allocation = allocations_[index];
            IndexTable& table = index_tables_[index];

            set_functions::remove(Allocation, table, target.id());
        }

        void removeAll(const Entity& target) {
            for (std::size_t i = 0; i < allocations_.size(); ++i) {
                IndexTable& table = index_tables_[i];

                if (!table.contains(target.id())) {
                    return;
                }

                Allocation& Allocation = allocations_[i];
                IndexType location = table[target.id()];

                const std::size_t stride = Allocation.stride();
                const std::size_t size = Allocation.size();

                std::size_t destination_offset = location * stride;
                std::size_t source_offset = size - stride;

                if (destination_offset < source_offset) {
                    std::byte* data = Allocation.data();
                    std::byte* destination = data + destination_offset;
                    std::byte* source = data + source_offset;

                    std::memmove(destination, source, stride);
                }

                Allocation.shrink(1);
                table.remove(target.id());
            }
        }

        template <traits::PODType... Ts>
        [[nodiscard]] View<Ts...> view() {
            return View<Ts...>{&index_tables_, &allocations_, &versions_};
        }

        template <traits::PODType... Ts>
        [[nodiscard]] ConstView<Ts...> view() const {
            return ConstView<Ts...>{&index_tables_, &allocations_, &versions_};
        }

    private:
        std::vector<Allocation> allocations_;
        std::vector<IndexTable> index_tables_;

        std::vector<IndexType> versions_;
        std::vector<IndexType> freeList_;
        std::vector<bool> statuses_;

        template <traits::PODType T>
        [[nodiscard]] static std::size_t typeIndex() {
            return traits::typeIndex<traits::ECSTypeContext, T>();
        }

        template <traits::PODType T>
        [[nodiscard]] std::size_t acquire() {
            std::size_t index = typeIndex<T>();

            if (index >= allocations_.size()) {
                allocations_.resize(std::max(4ul, index * 2));
                index_tables_.resize(std::max(4ul, index * 2));
            }

            if (!allocations_[index]) {
                allocations_[index].initialise(sizeof(T));
            }

            return index;
        }

        template <traits::PODType T>
        [[nodiscard]] bool acquirable() const {
            std::size_t index = typeIndex<T>();

            return index < allocations_.size();
        }
    };
}
