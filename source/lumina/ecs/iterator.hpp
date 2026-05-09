#pragma once

#include <lumina/meta/allocation.hpp>
#include <lumina/meta/index_table.hpp>
#include <lumina/meta/pod_type.hpp>
#include <lumina/meta/type_index.hpp>

#include "entity.hpp"
#include "set_functions.hpp"

namespace lumina::ecs {
    template <bool Const, bool Scanning, bool Expand, meta::PODType... Ts>
    class BasicComponentIterator final {
        using IndexType = Entity::ValueType;
        using IndexTable = meta::IndexTable<IndexType>;
        using IndexTables = const std::vector<IndexTable>;
        using Versions = const std::vector<IndexType>;
        using Allocation = meta::Allocation;
        using Allocations = std::conditional_t<Const, const std::vector<Allocation>, std::vector<Allocation>>;

        template <bool>
        struct Expander;

        template <>
        struct Expander<false> {
            using IteratorType = BasicComponentIterator<Const, Scanning, false, Ts...>;
            using Type = Entity;
            using InstanceType = std::conditional_t<Const, const IteratorType, IteratorType>;

            static Type get(auto& iterator) {
                auto& driver = (*iterator.tables_)[iterator.driverIndex_];
                auto index = static_cast<Entity::ValueType>(iterator.index_);
                auto id = driver.dense()[index];

                return Entity{
                    id,
                    (*iterator.versions_)[id],
                };
            }
        };

        template <>
        struct Expander<true> {
            using IteratorType = BasicComponentIterator<Const, Scanning, false, Ts...>;
            using Type = std::conditional_t<Const, const std::tuple<Entity, const Ts&...>, std::tuple<Entity, Ts&...>>;
            using InstanceType = std::conditional_t<Const, const IteratorType, IteratorType>;
            using IndexType = Entity::ValueType;

            template <meta::PODType T>
            using component_type = std::conditional_t<Const, const T, T>;

            static Type get(InstanceType& iterator) {
                auto& driver = (*iterator.tables_)[iterator.driverIndex_];
                auto index = static_cast<Entity::ValueType>(iterator.index_);
                auto id = driver.dense()[index];

                auto target = Entity{
                    id,
                    (*iterator.versions_)[id],
                };

                return Type{target, component_of<Ts>(id, iterator)...};
            }

            template <meta::PODType T>
            [[nodiscard]] static component_type<T>& component_of(IndexType id, InstanceType& iterator) {
                std::size_t index = meta::typeIndex<meta::ECSTypeContext, T>();

                return SetFunctions::get<T>((*iterator.allocations_)[index], (*iterator.tables_)[index], id);
            }
        };

        using ExpanderType = Expander<Expand>;

    public:
        // snake_case for C++ iterator_traits support
        using value_type = ExpanderType::Type;
        using reference = value_type;
        using pointer = value_type;
        using difference_type = std::ptrdiff_t;

        BasicComponentIterator(IndexTables* tables, Allocations* allocations, Versions* versions, std::size_t driver_index, std::size_t index, std::size_t end);

        ~BasicComponentIterator() = default;

        BasicComponentIterator(const BasicComponentIterator&) = default;
        BasicComponentIterator(BasicComponentIterator&&) noexcept = default;

        BasicComponentIterator& operator=(const BasicComponentIterator&) = default;
        BasicComponentIterator& operator=(BasicComponentIterator&&) noexcept = default;

        [[nodiscard]] reference operator*();
        [[nodiscard]] reference operator*() const;

        [[nodiscard]] bool operator==(const BasicComponentIterator&) const = default;
        [[nodiscard]] auto operator<=>(const BasicComponentIterator& other) const;

        BasicComponentIterator& operator+=(difference_type n);
        BasicComponentIterator& operator-=(difference_type n);

        [[nodiscard]] friend difference_type operator-(const BasicComponentIterator& a, const BasicComponentIterator& b) {
            return a.index_ - b.index_;
        }

        [[nodiscard]] friend auto operator+(BasicComponentIterator it, difference_type n) {
            return it += n;
        }

        [[nodiscard]] friend auto operator+(difference_type n, BasicComponentIterator it) {
            return it += n;
        }

        [[nodiscard]] friend auto operator-(BasicComponentIterator it, difference_type n) {
            return it -= n;
        }

        BasicComponentIterator& operator++();
        BasicComponentIterator operator++(int);

        BasicComponentIterator& operator--();
        BasicComponentIterator operator--(int);

    private:
        IndexTables* tables_;
        Allocations* allocations_;
        Versions* versions_;
        std::size_t driverIndex_;
        std::size_t index_;
        std::size_t end_;

        void scanForward();
        void scanBackward();

        template <meta::PODType T>
        [[nodiscard]] bool has(IndexType id);
        [[nodiscard]] bool hasAll(IndexType id);
    };

    template <bool Const, meta::PODType... Ts>
    using ComponentIterator = BasicComponentIterator<Const, false, false, Ts...>;

    template <bool Const, meta::PODType... Ts>
    using ScanningComponentIterator = BasicComponentIterator<Const, true, false, Ts...>;

    template <bool Const, meta::PODType... Ts>
    using ExpandingComponentIterator = BasicComponentIterator<Const, false, true, Ts...>;

    template <bool Const, meta::PODType... Ts>
    using ScanningExpandingComponentIterator = BasicComponentIterator<Const, true, true, Ts...>;
}