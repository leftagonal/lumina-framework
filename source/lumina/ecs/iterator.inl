#pragma once

#include "iterator.hpp"

#include <cstddef>
#include <iterator>

namespace lumina::ecs {
    template <AdvanceRule Advance, AccessRule Access, Component... Ts>
    Iterator<Advance, Access, Ts...>::Iterator(const IndexTables* indexTables, MemoryPools* memoryPools, const Versions* versions, std::size_t drivingIndex, std::size_t index, std::size_t end)
        : indexTables_(indexTables), memoryPools_(memoryPools), versions_(versions), drivingIndex_(drivingIndex), index_(index), end_(end) {
    }

    template <AdvanceRule Advance, AccessRule Access, Component... Ts>
    auto Iterator<Advance, Access, Ts...>::operator*() -> reference {
        return AccessorType::get(*memoryPools_, *indexTables_, *versions_, index_);
    }

    template <AdvanceRule Advance, AccessRule Access, Component... Ts>
    [[nodiscard]] auto Iterator<Advance, Access, Ts...>::operator<=>(const Iterator& other) const {
        return index_ <=> other.index_;
    }

    template <AdvanceRule Advance, AccessRule Access, Component... Ts>
    auto Iterator<Advance, Access, Ts...>::operator+=(difference_type n) -> Iterator& {
        while (n != 0) {
            ++(*this);
            --n;
        }

        return *this;
    }

    template <AdvanceRule Advance, AccessRule Access, Component... Ts>
    auto Iterator<Advance, Access, Ts...>::operator++() -> Iterator& {
        index_ = AdvancerType::nextIndex(*indexTables_, index_, end_);

        return *this;
    }

    template <AdvanceRule Advance, AccessRule Access, Component... Ts>
    auto Iterator<Advance, Access, Ts...>::operator++(int) -> Iterator {
        Iterator copy = *this;

        ++(*this);

        return copy;
    }
}
