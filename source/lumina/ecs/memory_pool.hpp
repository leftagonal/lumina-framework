#pragma once

#include <lumina/meta/exceptions.hpp>

#include <cstddef>
#include <cstring>
#include <vector>

namespace lumina::ecs {
    class MemoryPool final {
    public:
        MemoryPool() = default;
        ~MemoryPool() = default;

        MemoryPool(const MemoryPool&) = default;
        MemoryPool(MemoryPool&&) noexcept = default;

        MemoryPool& operator=(const MemoryPool&) = default;
        MemoryPool& operator=(MemoryPool&&) noexcept = default;

        [[nodiscard]] bool operator==(const MemoryPool& other) const {
            return memory_ == other.memory_;
        }

        [[nodiscard]] bool operator!=(const MemoryPool& other) const {
            return !(*this == other);
        }

        [[nodiscard]] std::byte* operator[](std::size_t index) {
            return memory_.data() + index;
        }

        [[nodiscard]] const std::byte* operator[](std::size_t index) const {
            return memory_.data() + index;
        }

        [[nodiscard]] std::byte* at(std::size_t index) {
            meta::assert(index < memory_.size(), "memory pool out-of-range access (max: {} given: {})", memory_.size() - 1, index);

            return memory_.data() + index;
        }

        [[nodiscard]] const std::byte* at(std::size_t index) const {
            meta::assert(index < memory_.size(), "memory pool out-of-range access (max: {} given: {})", memory_.size() - 1, index);

            return memory_.data() + index;
        }

        /**
         * @brief Allocates additional memory at the end of the pool.
         *
         * @param amount The number of bytes to allocate.
         * @returns std::byte* The start of the newly allocated memory.
         */
        std::byte* allocate(std::size_t amount) {
            memory_.resize(memory_.size() + amount);

            return &memory_[memory_.size() - amount];
        }

        /**
         * @brief Frees memory from the end of the pool.
         *
         * @param amount The number of bytes to free.
         */
        void free(std::size_t amount) {
            memory_.resize(memory_.size() - amount);
        }

        void moveBackTo(std::size_t index, std::size_t amount) {
            std::byte* destination = memory_.data() + index;
            std::byte* source = memory_.data() + memory_.size() - amount;

            std::memmove(destination, source, amount);
        }

        [[nodiscard]] std::size_t size() const {
            return memory_.size();
        }

        [[nodiscard]] std::byte* data() {
            return memory_.data();
        }

        [[nodiscard]] const std::byte* data() const {
            return memory_.data();
        }

        [[nodiscard]] bool empty() const {
            return memory_.empty();
        }

        void clear() {
            memory_.clear();
        }

    private:
        std::vector<std::byte> memory_;
    };

    using MemoryPools = std::vector<MemoryPool>;
}
