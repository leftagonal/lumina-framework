#pragma once

#include <lumina/ecs/iterator.hpp>

namespace lumina::ecs {
    template <Component... Ts>
    class View final {
        using IndexTables = const std::vector<IndexTable>;
        using Versions = const std::vector<std::size_t>;
        using MemoryPools = std::vector<MemoryPool>;

    public:
        class ImmediateIterable final {
        public:
            using Iterator = ScanningImmediateIterator<Ts...>;

            ImmediateIterable(IndexTables* indexTables, MemoryPools* memoryPools, Versions* versions, std::size_t drivingIndex);

            [[nodiscard]] Iterator begin();
            [[nodiscard]] Iterator end();

        private:
            IndexTables* indexTables_;
            MemoryPools* memoryPools_;
            Versions* versions_;
            std::size_t drivingIndex_;

            [[nodiscard]] std::size_t maximum() const;
        };

        class ExpandedIterable final {
        public:
            using Iterator = ScanningExpandingIterator<Ts...>;

            ExpandedIterable(IndexTables* indexTables, MemoryPools* memoryPools, Versions* versions, std::size_t drivingIndex);

            [[nodiscard]] Iterator begin();
            [[nodiscard]] Iterator end();

        private:
            IndexTables* indexTables_;
            MemoryPools* memoryPools_;
            Versions* versions_;
            std::size_t drivingIndex_;

            [[nodiscard]] std::size_t maximum() const;
        };

        View(IndexTables* indexTables, MemoryPools* memoryPools, Versions* versions);
        ~View() = default;

        [[nodiscard]] ImmediateIterable immediate();
        [[nodiscard]] ExpandedIterable expanded();

    private:
        IndexTables* indexTables_;
        MemoryPools* memoryPools_;
        Versions* versions_;

        [[nodiscard]] std::size_t getDriverIndex() const;

        template <Component T>
        void smallestOf(std::size_t& current) const;
    };
}
