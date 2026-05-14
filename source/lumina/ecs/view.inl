#pragma once

#include "lumina/ecs/rules.hpp"
#include "view.hpp"

namespace lumina::ecs {
    template <Component... Ts>
    View<Ts...>::View(IndexTables* indexTables, MemoryPools* memoryPools, Versions* versions)
        : indexTables_(indexTables), memoryPools_(memoryPools), versions_(versions) {
    }

    template <Component... Ts>
    View<Ts...>::ImmediateIterable::ImmediateIterable(IndexTables* indexTables, MemoryPools* memoryPools, Versions* versions, std::size_t drivingIndex)
        : indexTables_(indexTables), memoryPools_(memoryPools), versions_(versions), drivingIndex_(drivingIndex) {
    }

    template <Component... Ts>
    auto View<Ts...>::ImmediateIterable::begin() -> Iterator {
        return {
            indexTables_,
            memoryPools_,
            versions_,
            drivingIndex_,
            ScanningAdvance<Ts...>::firstIndex(*indexTables_, maximum()),
            maximum(),
        };
    }

    template <Component... Ts>
    auto View<Ts...>::ImmediateIterable::end() -> Iterator {
        return {
            indexTables_,
            memoryPools_,
            versions_,
            drivingIndex_,
            maximum(),
            maximum(),
        };
    }

    template <Component... Ts>
    std::size_t View<Ts...>::ImmediateIterable::maximum() const {
        return (*indexTables_)[drivingIndex_].dense().size();
    }

    template <Component... Ts>
    View<Ts...>::ExpandedIterable::ExpandedIterable(IndexTables* indexTables, MemoryPools* memoryPools, Versions* versions, std::size_t drivingIndex)
        : indexTables_(indexTables), memoryPools_(memoryPools), versions_(versions), drivingIndex_(drivingIndex) {
    }

    template <Component... Ts>
    auto View<Ts...>::ExpandedIterable::begin() -> Iterator {
        return {
            indexTables_,
            memoryPools_,
            versions_,
            drivingIndex_,
            ScanningAdvance<Ts...>::firstIndex(*indexTables_, maximum()),
            maximum(),
        };
    }

    template <Component... Ts>
    auto View<Ts...>::ExpandedIterable::end() -> Iterator {
        return {
            indexTables_,
            memoryPools_,
            versions_,
            drivingIndex_,
            maximum(),
            maximum(),
        };
    }

    template <Component... Ts>
    std::size_t View<Ts...>::ExpandedIterable::maximum() const {
        return (*indexTables_)[drivingIndex_].dense().size();
    }

    template <Component... Ts>
    auto View<Ts...>::immediate() -> ImmediateIterable {
        return {
            indexTables_,
            memoryPools_,
            versions_,
            getDriverIndex(),
        };
    }

    template <Component... Ts>
    auto View<Ts...>::expanded() -> ExpandedIterable {
        return {
            indexTables_,
            memoryPools_,
            versions_,
            getDriverIndex(),
        };
    }

    template <Component... Ts>
    std::size_t View<Ts...>::getDriverIndex() const {
        std::size_t current = 0xFFFFFFFFFFFFFFFFull;

        (..., smallestOf<Ts>(current));

        return current;
    }

    template <Component... Ts>
    template <Component T>
    void View<Ts...>::smallestOf(std::size_t& current) const {
        std::size_t index = typeIndex<T>();
        std::size_t size = (*indexTables_)[index].dense().size();
        std::size_t currentSize = (*indexTables_)[current].dense().size();

        if (size < currentSize) {
            current = index;
        }
    }
}
