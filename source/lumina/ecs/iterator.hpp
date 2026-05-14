#pragma once

#include "rules.hpp"

namespace lumina::ecs {
    template <AdvanceRule Advance, AccessRule Access, Component... Ts>
    class Iterator final {
        using AdvancerType = Advance;
        using AccessorType = Access;

    public:
        // snake_case for C++ iterator_traits support
        using value_type = AccessorType::Type;
        using reference = value_type;
        using pointer = value_type;
        using difference_type = std::ptrdiff_t;

        Iterator(const IndexTables* indexTables, MemoryPools* memoryPools, const Versions* versions, std::size_t drivingIndex, std::size_t index, std::size_t end);
        ~Iterator() = default;

        Iterator(const Iterator&) = default;
        Iterator(Iterator&&) noexcept = default;

        Iterator& operator=(const Iterator&) = default;
        Iterator& operator=(Iterator&&) noexcept = default;

        [[nodiscard]] reference operator*();

        [[nodiscard]] bool operator==(const Iterator&) const = default;
        [[nodiscard]] auto operator<=>(const Iterator& other) const;

        Iterator& operator+=(difference_type n);

        [[nodiscard]] friend difference_type operator-(const Iterator& a, const Iterator& b) {
            return a.index_ - b.index_;
        }

        [[nodiscard]] friend auto operator+(Iterator it, difference_type n) {
            return it += n;
        }

        [[nodiscard]] friend auto operator+(difference_type n, Iterator it) {
            return it += n;
        }

        Iterator& operator++();
        Iterator operator++(int);

    private:
        const IndexTables* indexTables_;
        MemoryPools* memoryPools_;
        const Versions* versions_;

        std::size_t drivingIndex_;
        std::size_t index_;
        std::size_t end_;
    };

    template <Component... Ts>
    using LinearImmediateIterator = Iterator<LinearAdvance, ImmediateAccess, Ts...>;

    template <Component... Ts>
    using ScanningImmediateIterator = Iterator<ScanningAdvance<Ts...>, ImmediateAccess, Ts...>;

    template <Component... Ts>
    using LinearExpandingIterator = Iterator<LinearAdvance, ExpandedAccess<Ts...>, Ts...>;

    template <Component... Ts>
    using ScanningExpandingIterator = Iterator<ScanningAdvance<Ts...>, ExpandedAccess<Ts...>, Ts...>;
}
