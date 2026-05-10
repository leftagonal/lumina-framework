#pragma once

#include "view.hpp"

namespace lumina::ecs {
    template <bool Const, Component... Ts>
    BasicView<Const, Ts...>::BasicView(IndexTables* tables, Allocations* allocations, Versions* versions)
        : tables_(tables), allocations_(allocations), versions_(versions) {
    }

    template <bool Const, Component... Ts>
    BasicView<Const, Ts...>::Iterable::Iterable(IndexTables* tables, Allocations* allocations, Versions* versions, std::size_t driver_index)
        : tables_(tables), allocations_(allocations), versions_(versions), driverIndex_(driver_index) {
    }

    template <bool Const, Component... Ts>
    auto BasicView<Const, Ts...>::Iterable::begin() -> Iterator requires(!Const)
    {
        return Iterator{
            tables_,
            allocations_,
            versions_,
            driverIndex_,
            0,
            maximum(),
        };
    }

    template <bool Const, Component... Ts>
    auto BasicView<Const, Ts...>::Iterable::end() -> Iterator requires(!Const)
    {
        return Iterator{
            tables_,
            allocations_,
            versions_,
            driverIndex_,
            maximum(),
            maximum(),
        };
    }

    template <bool Const, Component... Ts>
    auto BasicView<Const, Ts...>::Iterable::begin() const -> Iterator requires(Const)
    {
        return Iterator{
            tables_,
            allocations_,
            versions_,
            driverIndex_,
            0,
            maximum(),
        };
    }

    template <bool Const, Component... Ts>
    auto BasicView<Const, Ts...>::Iterable::end() const -> Iterator requires(Const)
    {
        return Iterator{
            tables_,
            allocations_,
            versions_,
            driverIndex_,
            maximum(),
            maximum(),
        };
    }

    template <bool Const, Component... Ts>
    std::size_t BasicView<Const, Ts...>::Iterable::maximum() const {
        return (*tables_)[driverIndex_].dense().size();
    }

    template <bool Const, Component... Ts>
    BasicView<Const, Ts...>::ExpandedIterable::ExpandedIterable(IndexTables* tables, Allocations* allocations, Versions* versions, std::size_t driver_index)
        : tables_(tables), allocations_(allocations), versions_(versions), driverIndex_(driver_index) {
    }

    template <bool Const, Component... Ts>
    auto BasicView<Const, Ts...>::ExpandedIterable::begin() -> Iterator requires(!Const)
    {
        return Iterator{
            tables_,
            allocations_,
            versions_,
            driverIndex_,
            0,
            maximum(),
        };
    }

    template <bool Const, Component... Ts>
    auto BasicView<Const, Ts...>::ExpandedIterable::end() -> Iterator requires(!Const)
    {
        return Iterator{
            tables_,
            allocations_,
            versions_,
            driverIndex_,
            maximum(),
            maximum(),
        };
    }

    template <bool Const, Component... Ts>
    auto BasicView<Const, Ts...>::ExpandedIterable::begin() const -> Iterator requires(Const)
    {
        return Iterator{
            tables_,
            allocations_,
            versions_,
            driverIndex_,
            0,
            maximum(),
        };
    }

    template <bool Const, Component... Ts>
    auto BasicView<Const, Ts...>::ExpandedIterable::end() const -> Iterator requires(Const)
    {
        return Iterator{
            tables_,
            allocations_,
            versions_,
            driverIndex_,
            maximum(),
            maximum(),
        };
    }

    template <bool Const, Component... Ts>
    std::size_t BasicView<Const, Ts...>::ExpandedIterable::maximum() const {
        return (*tables_)[driverIndex_].dense().size();
    }

    template <bool Const, Component... Ts>
    auto BasicView<Const, Ts...>::immediate() -> Iterable {
        return Iterable{
            tables_,
            allocations_,
            versions_,
            getDriverIndex(),
        };
    }

    template <bool Const, Component... Ts>
    auto BasicView<Const, Ts...>::expanded() -> ExpandedIterable {
        return ExpandedIterable{
            tables_,
            allocations_,
            versions_,
            getDriverIndex(),
        };
    }

    template <bool Const, Component... Ts>
    auto BasicView<Const, Ts...>::immediate() const -> Iterable {
        return Iterable{
            tables_,
            allocations_,
            versions_,
            getDriverIndex(),
        };
    }

    template <bool Const, Component... Ts>
    auto BasicView<Const, Ts...>::expanded() const -> ExpandedIterable {
        return ExpandedIterable{
            tables_,
            allocations_,
            versions_,
            getDriverIndex(),
        };
    }

    template <bool Const, Component... Ts>
    std::size_t BasicView<Const, Ts...>::getDriverIndex() const {
        std::size_t current = 0xFFFFFFFFFFFFFFFFull;

        (..., smallestOf<Ts>(current));

        return current;
    }

    template <bool Const, Component... Ts>
    template <Component T>
    void BasicView<Const, Ts...>::smallestOf(std::size_t& current) const {
        std::size_t index = typeIndex<T>();
        std::size_t size = (*tables_)[index].dense().size();

        current = std::min(current, size);
    }
}
