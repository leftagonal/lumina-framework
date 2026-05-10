#pragma once

#include <lumina/meta/type_index.hpp>

#include "iterator.hpp"
#include "set_functions.hpp"

#include <cstddef>
#include <iterator>

namespace lumina::ecs {
    template <bool Const, bool Scanning, bool Expand, Component... Ts>
    BasicComponentIterator<Const, Scanning, Expand, Ts...>::BasicComponentIterator(
        IndexTables* tables,
        Allocations* allocations,
        Versions* versions,
        std::size_t driverIndex,
        std::size_t index,
        std::size_t end)
        : tables_(tables), allocations_(allocations), versions_(versions),
          driverIndex_(driverIndex), index_(index), end_(end) {
    }

    template <bool Const, bool Scanning, bool Expand, Component... Ts>
    auto BasicComponentIterator<Const, Scanning, Expand, Ts...>::operator*() -> reference {
        return ExpanderType::get(*this);
    }

    template <bool Const, bool Scanning, bool Expand, Component... Ts>
    auto BasicComponentIterator<Const, Scanning, Expand, Ts...>::operator*() const -> reference {
        return ExpanderType::get(*this);
    }

    template <bool Const, bool Scanning, bool Expand, Component... Ts>
    [[nodiscard]] auto BasicComponentIterator<Const, Scanning, Expand, Ts...>::operator<=>(const BasicComponentIterator& other) const {
        return index_ <=> other.index_;
    }

    template <bool Const, bool Scanning, bool Expand, Component... Ts>
    auto BasicComponentIterator<Const, Scanning, Expand, Ts...>::operator+=(difference_type n) -> BasicComponentIterator& {
        while (n != 0) {
            ++(*this);
            --n;
        }

        return *this;
    }

    template <bool Const, bool Scanning, bool Expand, Component... Ts>
    auto BasicComponentIterator<Const, Scanning, Expand, Ts...>::operator-=(difference_type n) -> BasicComponentIterator& {
        while (n != 0) {
            --(*this);
            --n;
        }

        return *this;
    }

    template <bool Const, bool Scanning, bool Expand, Component... Ts>
    auto BasicComponentIterator<Const, Scanning, Expand, Ts...>::operator++() -> BasicComponentIterator& {
        ++index_;

        if constexpr (Scanning) {
            scanForward();
        }

        return *this;
    }

    template <bool Const, bool Scanning, bool Expand, Component... Ts>
    auto BasicComponentIterator<Const, Scanning, Expand, Ts...>::operator++(int) -> BasicComponentIterator {
        BasicComponentIterator copy = *this;

        ++(*this);

        return copy;
    }

    template <bool Const, bool Scanning, bool Expand, Component... Ts>
    auto BasicComponentIterator<Const, Scanning, Expand, Ts...>::operator--() -> BasicComponentIterator& {
        --index_;

        if constexpr (Scanning) {
            scanBackward();
        }

        return *this;
    }

    template <bool Const, bool Scanning, bool Expand, Component... Ts>
    auto BasicComponentIterator<Const, Scanning, Expand, Ts...>::operator--(int) -> BasicComponentIterator {
        BasicComponentIterator copy = *this;

        --(*this);

        return copy;
    }

    template <bool Const, bool Scanning, bool Expand, Component... Ts>
    void BasicComponentIterator<Const, Scanning, Expand, Ts...>::scanForward() {
        auto& driver = (*tables_)[driverIndex_];

        while (index_ != end_ && !hasAll(driver.dense()[index_])) {
            ++index_;
        }
    }

    template <bool Const, bool Scanning, bool Expand, Component... Ts>
    void BasicComponentIterator<Const, Scanning, Expand, Ts...>::scanBackward() {
        auto& driver = (*tables_)[driverIndex_];

        while (index_ != 0 && !hasAll(driver.dense()[index_])) {
            --index_;
        }
    }

    template <bool Const, bool Scanning, bool Expand, Component... Ts>
    template <Component T>
    [[nodiscard]] bool BasicComponentIterator<Const, Scanning, Expand, Ts...>::has(IndexType id) {
        std::size_t index = typeIndex<T>();

        return (*tables_)[index].contains(id);
    }

    template <bool Const, bool Scanning, bool Expand, Component... Ts>
    [[nodiscard]] bool BasicComponentIterator<Const, Scanning, Expand, Ts...>::hasAll(IndexType id) {
        return (... && has<Ts>(id));
    }
}
