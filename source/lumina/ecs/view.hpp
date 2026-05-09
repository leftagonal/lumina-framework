#pragma once

#include <lumina/ecs/iterator.hpp>

namespace lumina::ecs {
    template <bool Const, meta::PODType... Ts>
    class BasicView final {
        using IndexType = Entity::ValueType;
        using IndexTable = meta::IndexTable<IndexType>;
        using IndexTables = const std::vector<IndexTable>;
        using Versions = const std::vector<IndexType>;
        using Allocations = std::conditional_t<Const, const std::vector<meta::Allocation>, std::vector<meta::Allocation>>;

    public:
        BasicView(IndexTables* tables, Allocations* allocations, Versions* versions);

        class Iterable final {
            using Iterator = ScanningComponentIterator<Const, Ts...>;

        public:
            Iterable(IndexTables* tables, Allocations* allocations, Versions* versions, std::size_t driver_index);

            [[nodiscard]] Iterator begin() requires(!Const);
            [[nodiscard]] Iterator end() requires(!Const);
            [[nodiscard]] Iterator begin() const requires(Const);
            [[nodiscard]] Iterator end() const requires(Const);

        private:
            IndexTables* tables_;
            Allocations* allocations_;
            Versions* versions_;
            std::size_t driverIndex_;

            [[nodiscard]] std::size_t maximum() const;
        };

        class ExpandedIterable final {
            using Iterator = ScanningExpandingComponentIterator<Const, Ts...>;

        public:
            ExpandedIterable(IndexTables* tables, Allocations* allocations, Versions* versions, std::size_t driver_index);

            [[nodiscard]] Iterator begin() requires(!Const);
            [[nodiscard]] Iterator end() requires(!Const);
            [[nodiscard]] Iterator begin() const requires(Const);
            [[nodiscard]] Iterator end() const requires(Const);

        private:
            IndexTables* tables_;
            Allocations* allocations_;
            Versions* versions_;
            std::size_t driverIndex_;

            [[nodiscard]] std::size_t maximum() const;
        };

        [[nodiscard]] Iterable immediate();
        [[nodiscard]] ExpandedIterable expanded();

        [[nodiscard]] Iterable immediate() const;
        [[nodiscard]] ExpandedIterable expanded() const;

    private:
        IndexTables* tables_;
        Allocations* allocations_;
        Versions* versions_;

        [[nodiscard]] std::size_t getDriverIndex() const;

        template <meta::PODType T>
        void smallestOf(std::size_t& current) const;
    };

    template <meta::PODType... Ts>
    using View = BasicView<false, Ts...>;

    template <meta::PODType... Ts>
    using ConstView = BasicView<true, Ts...>;
}