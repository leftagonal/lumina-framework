#pragma once

#include <lumina/traits/pod_type.hpp>
#include <lumina/traits/type_index.hpp>

#include <lumina/ecs/entity.hpp>
#include <lumina/ecs/set_functions.hpp>

#include <lumina/structs/allocation.hpp>
#include <lumina/structs/index_table.hpp>

#include <cstddef>
#include <iterator>

namespace lumina::ecs {
    template <bool Const, bool Scanning, bool Expand, traits::PODType... Ts>
    class BasicComponentIterator final {
        using IndexType = Entity::ValueType;

        template <bool>
        struct ExpandRule;

        template <>
        struct ExpandRule<false> {
            using Type = Entity;
            using InstanceType = std::conditional_t<Const, const BasicComponentIterator, BasicComponentIterator>;

            static Type get(InstanceType& iterator) {
                auto& driver = (*iterator.tables_)[iterator.driver_index_];
                auto index = static_cast<Entity::ValueType>(iterator.index_);
                auto id = driver.dense()[index];

                return Entity{
                    id,
                    (*iterator.versions_)[id],
                };
            }
        };

        template <>
        struct ExpandRule<true> {
            using Type = std::conditional_t<Const, const std::tuple<Entity, const Ts&...>, std::tuple<Entity, Ts&...>>;
            using InstanceType = std::conditional_t<Const, const BasicComponentIterator, BasicComponentIterator>;

            template <traits::PODType T>
            using component_type = std::conditional_t<Const, const T, T>;

            static Type get(InstanceType& iterator) {
                auto& driver = (*iterator.tables_)[iterator.driver_index_];
                auto index = static_cast<Entity::ValueType>(iterator.index_);
                auto id = driver.dense()[index];

                auto target = Entity{
                    id,
                    (*iterator.versions_)[id],
                };

                return Type{target, component_of<Ts>(id, iterator)...};
            }

            template <traits::PODType T>
            [[nodiscard]] static component_type<T>& component_of(IndexType id, InstanceType& iterator) {
                std::size_t index = traits::typeIndex<traits::ECSTypeContext, T>();

                return set_functions::get<T>((*iterator.allocations_)[index], (*iterator.tables_)[index], id);
            }
        };

        using IndexTable = structs::IndexTable<IndexType>;
        using IndexTables = const std::vector<IndexTable>;
        using Versions = const std::vector<IndexType>;
        using Allocations = std::conditional_t<Const, const std::vector<structs::Allocation>, std::vector<structs::Allocation>>;

    public:
        // snake_case for C++ iterator_traits support
        using value_type = ExpandRule<Expand>::Type;
        using reference = value_type;
        using pointer = value_type;
        using difference_type = std::ptrdiff_t;

        BasicComponentIterator(IndexTables* tables, Allocations* allocations, Versions* versions, std::size_t driver_index, std::size_t index, std::size_t end)
            : tables_(tables), allocations_(allocations), versions_(versions), driver_index_(driver_index), index_(index), end_(end) {
        }

        ~BasicComponentIterator() = default;

        BasicComponentIterator(const BasicComponentIterator&) = default;
        BasicComponentIterator(BasicComponentIterator&&) noexcept = default;

        BasicComponentIterator& operator=(const BasicComponentIterator&) = default;
        BasicComponentIterator& operator=(BasicComponentIterator&&) noexcept = default;

        [[nodiscard]] reference operator*() {
            return ExpandRule<Expand>::get(*this);
        }

        [[nodiscard]] reference operator*() const {
            return ExpandRule<Expand>::get(*this);
        }

        [[nodiscard]] friend difference_type operator-(const BasicComponentIterator& a, const BasicComponentIterator& b) {
            return a.index_ - b.index_;
        }

        [[nodiscard]] bool operator==(const BasicComponentIterator&) const = default;

        [[nodiscard]] auto operator<=>(const BasicComponentIterator& other) const {
            return index_ <=> other.index_;
        }

        BasicComponentIterator& operator+=(difference_type n) {
            while (n != 0) {
                ++(*this);
                --n;
            }

            return *this;
        }

        BasicComponentIterator& operator-=(difference_type n) {
            while (n != 0) {
                --(*this);
                --n;
            }

            return *this;
        }

        [[nodiscard]] friend BasicComponentIterator operator+(BasicComponentIterator it, difference_type n) {
            return it += n;
        }

        [[nodiscard]] friend BasicComponentIterator operator+(difference_type n, BasicComponentIterator it) {
            return it += n;
        }

        [[nodiscard]] friend BasicComponentIterator operator-(BasicComponentIterator it, difference_type n) {
            return it -= n;
        }

        BasicComponentIterator& operator++() {
            ++index_;

            if constexpr (Scanning) {
                scanForward();
            }

            return *this;
        }

        BasicComponentIterator operator++(int) {
            BasicComponentIterator copy = *this;

            ++(*this);

            return copy;
        }

        BasicComponentIterator& operator--() {
            --index_;

            if constexpr (Scanning) {
                scanBackward();
            }

            return *this;
        }

        BasicComponentIterator operator--(int) {
            BasicComponentIterator copy = *this;

            --(*this);

            return copy;
        }

    private:
        IndexTables* tables_;
        Allocations* allocations_;
        Versions* versions_;
        std::size_t driver_index_;
        std::size_t index_;
        std::size_t end_;

        void scanForward() {
            auto& driver = (*tables_)[driver_index_];

            while (index_ != end_ && !hasAll(driver.dense()[index_])) {
                ++index_;
            }
        }

        void scanBackward() {
            auto& driver = (*tables_)[driver_index_];

            while (index_ != 0 && !hasAll(driver.dense()[index_])) {
                --index_;
            }
        }

        template <traits::PODType T>
        [[nodiscard]] bool has(IndexType id) {
            std::size_t index = traits::typeIndex<traits::ECSTypeContext, T>();

            return (*tables_)[index].contains(id);
        }

        [[nodiscard]] bool hasAll(IndexType id) {
            return (... && has<Ts>(id));
        }
    };

    template <bool Const, traits::PODType... Ts>
    using ComponentIterator = BasicComponentIterator<Const, false, false, Ts...>;

    template <bool Const, traits::PODType... Ts>
    using ScanningComponentIterator = BasicComponentIterator<Const, true, false, Ts...>;

    template <bool Const, traits::PODType... Ts>
    using ExpandingComponentIterator = BasicComponentIterator<Const, false, true, Ts...>;

    template <bool Const, traits::PODType... Ts>
    using ScanningExpandingComponentIterator = BasicComponentIterator<Const, true, true, Ts...>;
}