#pragma once

#include "meta.hpp"
#include "index_table.hpp"
#include "memory_pool.hpp"

namespace lumina::ecs {
    template <Component T>
    class SparseSet {
    public:
        using Type = T;

        SparseSet() = delete;

        [[nodiscard]] static T& at(MemoryPool& memoryPool, const IndexTable& indexTable, std::size_t index) {
            auto* memory = memoryPool[indexTable[index] * sizeof(T)];
            auto* candidate = std::launder(reinterpret_cast<T*>(memory));

            return *candidate;
        }

        [[nodiscard]] static const T& at(const MemoryPool& memoryPool, const IndexTable& indexTable, std::size_t index) {
        	auto* memory = memoryPool[indexTable[index] * sizeof(T)];
            auto* candidate = std::launder(reinterpret_cast<const T*>(memory));

            return *candidate;
        }

        static T& insert(MemoryPool& memoryPool, IndexTable& indexTable, std::size_t index) {
            indexTable.insert(index);
            memoryPool.allocate(sizeof(T));

            return at(memoryPool, indexTable, index);
        }

        static T& insert(MemoryPool& memoryPool, IndexTable& indexTable, std::size_t index, const T& value) {
            T& element = insert(memoryPool, indexTable, index);
            ::new (&element) T(value);

            return element;
        }

        static T& insert(MemoryPool& memoryPool, IndexTable& indexTable, std::size_t index, T&& value) {
            T& element = insert(memoryPool, indexTable, index);
            ::new (&element) T(value);

            return element;
        }

        static void remove(MemoryPool& memoryPool, IndexTable& indexTable, std::size_t index) {
            std::size_t endIndex = indexTable.dense().back();

            if (endIndex != index) {
                std::size_t dense = indexTable[index];

                indexTable.moveBackTo(index);
                memoryPool.moveBackTo(dense, sizeof(T));
            }

            indexTable.pop();
            memoryPool.free(sizeof(T));
        }
    };
}
