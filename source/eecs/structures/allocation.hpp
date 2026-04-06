#pragma once

#include <algorithm>
#include <cstddef>

namespace eecs {
    class allocation final {
    public:
        allocation() = default;

        ~allocation() {
            deallocate();
        }

        allocation(const allocation& other) {
            if (!other) {
                data_ = nullptr;
                size_ = 0;

                return;
            }

            size_ = other.size();
            data_ = new std::byte[size_];

            std::memcpy(data_, other.data(), size_);
        }

        allocation(allocation&& other) noexcept {
            size_ = other.size();
            data_ = other.data();

            other.size_ = 0;
            other.data_ = nullptr;
        }

        allocation& operator=(const allocation& other) {
            if (&other == this) {
                return *this;
            }

            if (!other) {
                deallocate();

                return *this;
            }

            size_ = other.size();
            data_ = new std::byte[size_];

            std::memcpy(data_, other.data(), size_);

            return *this;
        }

        allocation& operator=(allocation&& other) noexcept {
            if (&other == this) {
                return *this;
            }

            size_ = other.size();
            data_ = other.data();

            other.size_ = 0;
            other.data_ = nullptr;

            return *this;
        }

        [[nodiscard]] bool operator==(const allocation& other) const {
            if (size_ != other.size()) {
                return false;
            }

            return std::memcmp(data_, other.data(), size_) == 0;
        }

        [[nodiscard]] bool operator!=(const allocation& other) const {
            return !(*this == other);
        }

        [[nodiscard]] std::byte& operator[](std::size_t index) {
            return data_[index];
        }

        [[nodiscard]] const std::byte& operator[](std::size_t index) const {
            return data_[index];
        }

        void allocate(std::size_t bytes) {
            deallocate();

            if (bytes == 0) {
                return;
            }

            size_ = bytes;
            data_ = new std::byte[size_];
        }

        void extend(std::size_t bytes) {
            if (bytes == 0) {
                return;
            }

            if (!allocated()) {
                allocate(bytes);

                return;
            }

            std::byte* old_data = data_;
            std::size_t old_size = size_;

            size_ += bytes;
            data_ = new std::byte[size_];

            std::memcpy(data_, old_data, old_size);

            delete[] old_data;
        }

        void shrink(std::size_t bytes) {
            if (bytes == 0 || !allocated()) {
                return;
            }

            if (bytes == size_) {
                deallocate();
            }

            std::byte* old_data = data_;

            size_ -= bytes;
            data_ = new std::byte[size_];

            std::memcpy(data_, old_data, size_);

            delete[] old_data;
        }

        [[nodiscard]] bool allocated() const {
            return data_ != nullptr;
        }

        explicit operator bool() const {
            return allocated();
        }

        [[nodiscard]] std::size_t size() const {
            return size_;
        }

        [[nodiscard]] std::byte* data() {
            return data_;
        }

        [[nodiscard]] const std::byte* data() const {
            return data_;
        }

        void deallocate() {
            if (data_) {
                return;
            }

            delete[] data_;

            data_ = nullptr;
            size_ = 0;
        }

    private:
        std::byte* data_ = nullptr;
        std::size_t size_ = 0;
    };
}
