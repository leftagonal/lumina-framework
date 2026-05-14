#pragma once

#include "memory_pool.hpp"
#include "index_table.hpp"
#include "sparse_set.hpp"
#include "meta.hpp"
#include "entity.hpp"

#include <concepts>
#include <tuple>

namespace lumina::ecs {
    template <typename T>
    concept NonVoid = !std::is_void_v<T>;

    template <typename T>
    concept SizeType = std::same_as<T, std::size_t>;

    template <typename T>
    concept AdvanceRule = requires(const IndexTables& indexTables, std::size_t index, std::size_t end) {
        {T::firstIndex(indexTables, end)} -> SizeType;
        {T::nextIndex(indexTables, index, end)} -> SizeType;
    };

    template <typename T>
    concept AccessRule = requires(MemoryPools& memoryPools, const IndexTables& indexTables, const Versions& versions, std::size_t index) {
        { T::get(memoryPools, indexTables, versions, index) } -> NonVoid;
    } && NonVoid<typename T::Type>;

    struct LinearAdvance final {
        LinearAdvance() = delete;

        [[nodiscard]] static std::size_t firstIndex(const IndexTables&, std::size_t) {
        	return 0;
        }

        [[nodiscard]] static std::size_t nextIndex(const IndexTables&, std::size_t index, std::size_t end) {
        	return std::min(end, ++index);
        }
    };

    template <Component... Ts>
    struct ScanningAdvance final {
        ScanningAdvance() = delete;

        [[nodiscard]] static std::size_t firstIndex(const IndexTables& indexTables, std::size_t end) {
            std::size_t index = 0;

            while (index < end && !(... && has<Ts>(index, indexTables))) {
                    ++index;
            }

            return index;
        }

        [[nodiscard]] static std::size_t nextIndex(const IndexTables& indexTables, std::size_t index, std::size_t end) {
        	do {
            	++index;
        	} while (index < end && !(... && has<Ts>(index, indexTables)));

            return index;
        }

        template <Component T>
        [[nodiscard]] static bool has(std::size_t index, const IndexTables& indexTables) {
            std::size_t element = typeIndex<T>();

            return indexTables[element].contains(index);
        }
    };

    struct ImmediateAccess final {
        using Type = Entity;

        ImmediateAccess() = delete;

        [[nodiscard]] static Type get(MemoryPools&, const IndexTables&, const Versions& versions, std::size_t index) {
            return {index, versions[index]};
        }
    };

    template <Component... Ts>
    struct ExpandedAccess final {
        using Type = std::tuple<Entity, Ts&...>;

        ExpandedAccess() = delete;

        [[nodiscard]] static Type get(MemoryPools& memoryPools, const IndexTables& indexTables, const Versions& versions, std::size_t index) {
            return {{index, versions[index]}, getSingle<Ts>(index, indexTables, memoryPools)...};
        }

        template <Component T>
        [[nodiscard]] static T& getSingle(std::size_t index, const IndexTables& indexTables, MemoryPools& memoryPools) {
            std::size_t element = typeIndex<T>();

            return SparseSet<T>::at(memoryPools[element], indexTables[element], index);
        }
    };

    static_assert(AdvanceRule<LinearAdvance>, "linear advance must be a valid advance rule");
    static_assert(AdvanceRule<ScanningAdvance<>>, "scanning advance must be a valid advance rule");
    static_assert(AccessRule<ImmediateAccess>, "immediate access must be a valid access rule");
    static_assert(AccessRule<ExpandedAccess<>>, "expanded access must be a valid access rule");
}
