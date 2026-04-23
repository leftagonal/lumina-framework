#pragma once

#include <vesta/iterators/striding_iterator.hpp>

#include <algorithm>
#include <cstddef>

namespace vesta::internal {
    /**
     * @brief A resizable region of memory.
     *
     */
    class allocation final {
    public:
        using iterator = striding_iterator;
        using const_iterator = const_striding_iterator;

        allocation() = default;

        /**
         * @brief Construct a new allocation object.
         *
         * @param stride The stride to allocate with.
         */
        allocation(std::size_t stride)
            : stride_(std::max(stride, 1ul)) {
        }

        /**
         * @brief Destroy the allocation object.
         *
         */
        ~allocation() {
            reset();
        }

        /**
         * @brief Construct a new allocation object.
         *
         * @param other The allocation to copy data from.
         */
        allocation(const allocation& other) {
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
         * @brief Construct a new allocation object.
         *
         * @param other The allocation to move data from.
         */
        allocation(allocation&& other) noexcept {
            capacity_ = other.capacity_;
            size_ = other.size_;
            data_ = other.data_;

            other.capacity_ = 0;
            other.size_ = 0;
            other.data_ = nullptr;
        }

        /**
         * @brief Copy another allocation into this allocation.
         *
         * @param other The allocation to copy data from.
         * @return allocation& A reference to this allocation.
         */
        allocation& operator=(const allocation& other) {
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
         * @brief Move another allocation into this allocation.
         *
         * @param other The allocation to move data from.
         * @return allocation& A reference to this allocation.
         */
        allocation& operator=(allocation&& other) noexcept {
            if (&other == this) {
                return *this;
            }

            stride_ = other.stride_;
            capacity_ = other.capacity_;
            size_ = other.size_;
            data_ = other.data_;

            // don't null stride, it must be > 1 anyway

            other.capacity_ = 0;
            other.size_ = 0;
            other.data_ = nullptr;

            return *this;
        }

        /**
         * @brief Compare two allocations for equality.
         *
         * @param other The allocation to compare against.
         * @return true If the two allocations hold the same data.
         * @return false If the two allocations do not hold the same data.
         */
        [[nodiscard]] bool operator==(const allocation& other) const {
            if (size_ != other.size_) {
                return false;
            }

            return std::memcmp(data_, other.data_, size_) == 0;
        }

        /**
         * @brief Compare two allocations for inequality.
         *
         * @param other The allocation to compare against.
         * @return true If the two allocations do not hold the same data.
         * @return false If the two allocations hold the same data.
         */
        [[nodiscard]] bool operator!=(const allocation& other) const {
            return !(*this == other);
        }

        /**
         * @brief Access memory of the allocation.
         *
         * @param index The index of the aligned block to access.
         * @return std::byte& A reference to the accessed memory.
         */
        [[nodiscard]] std::byte& operator[](std::size_t index) {
            return data_[index * stride_];
        }

        /**
         * @brief Access memory of the allocation.
         *
         * @param index The index of the aligned block to access.
         * @return std::byte& A reference to the accessed memory.
         */
        [[nodiscard]] const std::byte& operator[](std::size_t index) const {
            return data_[index * stride_];
        }

        /**
         * @brief Extend the allocation.
         *
         * @param count The number of strides to extend the allocation by.
         */
        void extend(std::size_t count) {
            if (count == 0) {
                return;
            }

            std::size_t needed = size_ + (count * stride_);

            if (capacity_ < needed) {
                std::size_t new_capacity = std::max(needed * 2, capacity_ * 2);
                std::byte* new_data = allocate(new_capacity);

                if (data_) {
                    std::memcpy(new_data, data_, size_);
                    free(data_);
                }

                data_ = new_data;
                capacity_ = new_capacity;
            }

            size_ = needed;
        }

        /**
         * @brief Shrink the allocation.
         *
         * @param count The number of strides to shrink the allocation by.
         */
        void shrink(std::size_t count) {
            std::size_t delta = count * stride_;
            size_ = (delta < size_) ? size_ - delta : 0;
        }

        /**
         * @brief Completely deallocates the allocation.
         *
         * This will invalidate the allocation.
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
         * @brief Prepares the allocation for usage.
         *
         * @param stride The stride to use.
         *
         * The allocation must already be invalidated before initialisation.
         */
        void initialise(std::size_t stride) {
            stride_ = stride;
        }

        /**
         * @brief Clear the allocation.
         *
         * This does not invalidate the allocation.
         */
        void clear() {
            size_ = 0;
        }

        /**
         * @brief Returns whether the allocation holds no meaningful data.
         *
         * @return true If the allocation holds accessible data.
         * @return false If the allocation does not hold accessible data.
         */
        [[nodiscard]] bool empty() const {
            return data_ == nullptr;
        }

        /**
         * @brief Returns whether the allocation is ready to be allocated.
         *
         * @return true If the allocation is ready for allocation.
         * @return false If the allocation is not ready for allocation.
         */
        [[nodiscard]] bool valid() const {
            return stride_ != 0;
        }

        /**
         * @brief Returns whether the allocation is ready to be allocated.
         *
         * @return true If the allocation is ready for allocation.
         * @return false If the allocation is not ready for allocation.
         */
        explicit operator bool() const {
            return !valid();
        }

        /**
         * @brief Returns the size of the addressable space of the allocation in bytes.
         *
         * @return std::size_t The addressable space size of the allocation.
         */
        [[nodiscard]] std::size_t size() const {
            return size_;
        }

        /**
         * @brief Returns the size of the allocation's allocated space, including non-addressable space.
         *
         * @return std::size_t The entire capacity of the allocation.
         */
        [[nodiscard]] std::size_t capacity() const {
            return capacity_;
        }

        /**
         * @brief Returns the allocation stride of the allocation.
         *
         * @return std::size_t The stride of the allocation.
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
         * @brief Returns a pointer to the allocation's memory.
         *
         * @return std::byte* The allocation's memory.
         */
        [[nodiscard]] std::byte* data() {
            return data_;
        }

        /**
         * @brief Returns a pointer to the allocation's memory.
         *
         * @return std::byte* The allocation's memory.
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