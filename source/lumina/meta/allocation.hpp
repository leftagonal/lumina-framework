#pragma once

#include "striding_iterator.hpp"

#include <algorithm>
#include <cstddef>

namespace lumina::meta {
    /**
     * @brief A resizable region of memory.
     *
     */
    class Allocation final {
    public:
        using iterator = StridingIterator<std::byte>;
        using const_iterator = ConstStridingIterator<std::byte>;

        Allocation() = default;

        /**
         * @brief Construct a new Allocation object.
         *
         * @param stride The stride to allocate with.
         */
        Allocation(std::size_t stride)
            : stride_(std::max(stride, 1ul)) {
        }

        /**
         * @brief Destroy the Allocation object.
         *
         */
        ~Allocation() {
            reset();
        }

        /**
         * @brief Construct a new Allocation object.
         *
         * @param other The Allocation to copy data from.
         */
        Allocation(const Allocation& other) {
            stride_ = other.stride_;

            if (!other) {
                return;
            }

            capacity_ = other.capacity_;
            size_ = other.size_;
            data_ = allocate(capacity_);

            std::memcpy(data_, other.data_, size_);
        }

        /**
         * @brief Construct a new Allocation object.
         *
         * @param other The Allocation to move data from.
         */
        Allocation(Allocation&& other) noexcept {
            capacity_ = other.capacity_;
            stride_ = other.stride_;
            size_ = other.size_;
            data_ = other.data_;

            other.stride_ = 0;
            other.capacity_ = 0;
            other.size_ = 0;
            other.data_ = nullptr;
        }

        /**
         * @brief Copy another Allocation into this Allocation.
         *
         * @param other The Allocation to copy data from.
         * @return Allocation& A reference to this Allocation.
         */
        Allocation& operator=(const Allocation& other) {
            if (&other == this) {
                return *this;
            }

            if (!other) {
                reset();

                return *this;
            }

            if (data_) {
                free(data_);
            }

            stride_ = other.stride_;
            capacity_ = other.capacity_;
            size_ = other.size_;
            data_ = allocate(capacity_);

            std::memcpy(data_, other.data(), size_);

            return *this;
        }

        /**
         * @brief Move another Allocation into this Allocation.
         *
         * @param other The Allocation to move data from.
         * @return Allocation& A reference to this Allocation.
         */
        Allocation& operator=(Allocation&& other) noexcept {
            if (&other == this) {
                return *this;
            }

            stride_ = other.stride_;
            capacity_ = other.capacity_;
            size_ = other.size_;
            data_ = other.data_;

            // don't null stride, it must be > 1 anyway

            other.stride_ = 0;
            other.capacity_ = 0;
            other.size_ = 0;
            other.data_ = nullptr;

            return *this;
        }

        /**
         * @brief Compare two Allocations for equality.
         *
         * @param other The Allocation to compare against.
         * @return true If the two Allocations hold the same data.
         * @return false If the two Allocations do not hold the same data.
         */
        [[nodiscard]] bool operator==(const Allocation& other) const {
            if (size_ != other.size_) {
                return false;
            }

            return std::memcmp(data_, other.data_, size_) == 0;
        }

        /**
         * @brief Compare two Allocations for inequality.
         *
         * @param other The Allocation to compare against.
         * @return true If the two Allocations do not hold the same data.
         * @return false If the two Allocations hold the same data.
         */
        [[nodiscard]] bool operator!=(const Allocation& other) const {
            return !(*this == other);
        }

        /**
         * @brief Access memory of the Allocation.
         *
         * @param index The index of the aligned block to access.
         * @return std::byte& A reference to the accessed memory.
         */
        [[nodiscard]] std::byte& operator[](std::size_t index) {
            return data_[index * stride_];
        }

        /**
         * @brief Access memory of the Allocation.
         *
         * @param index The index of the aligned block to access.
         * @return std::byte& A reference to the accessed memory.
         */
        [[nodiscard]] const std::byte& operator[](std::size_t index) const {
            return data_[index * stride_];
        }

        /**
         * @brief Extend the Allocation.
         *
         * @param count The number of strides to extend the Allocation by.
         */
        void extend(std::size_t count) {
            if (count == 0) {
                return;
            }

            std::size_t needed = size_ + (count * stride_);

            if (capacity_ < needed) {
                std::size_t newCapacity = std::max(needed * 2, capacity_ * 2);
                std::byte* newData = allocate(newCapacity);

                if (data_) {
                    std::memcpy(newData, data_, size_);
                    free(data_);
                }

                data_ = newData;
                capacity_ = newCapacity;
            }

            size_ = needed;
        }

        /**
         * @brief Shrink the Allocation.
         *
         * @param count The number of strides to shrink the Allocation by.
         */
        void shrink(std::size_t count) {
            std::size_t delta = count * stride_;
            size_ = (delta < size_) ? size_ - delta : 0;
        }

        /**
         * @brief Completely deallocates the Allocation.
         *
         * This will invalidate the Allocation.
         */
        void reset() {
            if (data_) {
                free(data_);

                data_ = nullptr;
                size_ = 0;
                capacity_ = 0;
                stride_ = 0;
            }
        }

        /**
         * @brief Prepares the Allocation for usage.
         *
         * @param stride The stride to use.
         *
         * The Allocation must already be invalidated before initialisation.
         */
        void initialise(std::size_t stride) {
            stride_ = stride;
        }

        /**
         * @brief Clear the Allocation.
         *
         * This does not invalidate the Allocation.
         */
        void clear() {
            size_ = 0;
        }

        /**
         * @brief Returns whether the Allocation holds no meaningful data.
         *
         * @return true If the Allocation holds accessible data.
         * @return false If the Allocation does not hold accessible data.
         */
        [[nodiscard]] bool empty() const {
            return data_ == nullptr;
        }

        /**
         * @brief Returns whether the Allocation is ready to be allocated.
         *
         * @return true If the Allocation is ready for Allocation.
         * @return false If the Allocation is not ready for Allocation.
         */
        [[nodiscard]] bool valid() const {
            return stride_ != 0;
        }

        /**
         * @brief Returns whether the Allocation is ready to be allocated.
         *
         * @return true If the Allocation is ready for Allocation.
         * @return false If the Allocation is not ready for Allocation.
         */
        explicit operator bool() const {
            return !valid();
        }

        /**
         * @brief Returns the size of the addressable space of the Allocation in bytes.
         *
         * @return std::size_t The addressable space size of the Allocation.
         */
        [[nodiscard]] std::size_t size() const {
            return size_;
        }

        /**
         * @brief Returns the size of the Allocation's allocated space, including non-addressable space.
         *
         * @return std::size_t The entire capacity of the Allocation.
         */
        [[nodiscard]] std::size_t capacity() const {
            return capacity_;
        }

        /**
         * @brief Returns the Allocation stride of the Allocation.
         *
         * @return std::size_t The stride of the Allocation.
         */
        [[nodiscard]] std::size_t stride() const {
            return stride_;
        }

        /**
         * @brief Returns the number of aligned regions of memory.
         *
         * @return std::size_t The number of aligned regions of memory.
         */
        [[nodiscard]] std::size_t count() const {
            return size_ / stride_;
        }

        /**
         * @brief Returns a pointer to the Allocation's memory.
         *
         * @return std::byte* The Allocation's memory.
         */
        [[nodiscard]] std::byte* data() {
            return data_;
        }

        /**
         * @brief Returns a pointer to the Allocation's memory.
         *
         * @return std::byte* The Allocation's memory.
         */
        [[nodiscard]] const std::byte* data() const {
            return data_;
        }

        [[nodiscard]] iterator begin() {
            return iterator{
                data_,
                static_cast<iterator::difference_type>(stride_),
            };
        }

        [[nodiscard]] iterator end() {
            return iterator{
                data_ + size_,
                static_cast<iterator::difference_type>(stride_),
            };
        }

        [[nodiscard]] const_iterator cbegin() const {
            return const_iterator{
                data_,
                static_cast<iterator::difference_type>(stride_),
            };
        }

        [[nodiscard]] const_iterator cend() const {
            return const_iterator{
                data_ + size_,
                static_cast<iterator::difference_type>(stride_),
            };
        }

        [[nodiscard]] const_iterator begin() const {
            return const_iterator{
                data_,
                static_cast<iterator::difference_type>(stride_),
            };
        }

        [[nodiscard]] const_iterator end() const {
            return const_iterator{
                data_ + size_,
                static_cast<iterator::difference_type>(stride_),
            };
        }

        [[nodiscard]] auto rbegin() {
            return std::reverse_iterator(begin());
        }

        [[nodiscard]] auto rend() {
            return std::reverse_iterator(end());
        }

        [[nodiscard]] auto rbegin() const {
            return std::reverse_iterator(begin());
        }

        [[nodiscard]] auto rend() const {
            return std::reverse_iterator(end());
        }

        [[nodiscard]] auto crbegin() const {
            return std::reverse_iterator(cbegin());
        }

        [[nodiscard]] auto crend() const {
            return std::reverse_iterator(cend());
        }

    private:
        std::byte* data_ = nullptr;

        std::size_t size_ = 0;
        std::size_t capacity_ = 0;
        std::size_t stride_ = 1;

        [[nodiscard]] static std::byte* allocate(std::size_t bytes) {
            return reinterpret_cast<std::byte*>(malloc(bytes));
        }
    };
}