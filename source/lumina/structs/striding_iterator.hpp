#pragma once

#include <cstddef>
#include <iterator>

namespace lumina::structs {
    template <typename T, bool Const>
    class BasicStridingIterator final {
    public:
        // snake_case for C++ iterator_traits support
        using value_type = T;
        using reference = std::conditional_t<Const, const value_type, value_type>&;
        using pointer = std::conditional_t<Const, const value_type, value_type>*;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;

        BasicStridingIterator(std::byte* current, difference_type stride)
            : current_(current), stride_(stride) {
        }

        ~BasicStridingIterator() = default;

        BasicStridingIterator(const BasicStridingIterator&) = default;
        BasicStridingIterator(BasicStridingIterator&&) noexcept = default;

        BasicStridingIterator& operator=(const BasicStridingIterator&) = default;
        BasicStridingIterator& operator=(BasicStridingIterator&&) noexcept = default;

        [[nodiscard]] pointer operator->() {
            return std::launder(reinterpret_cast<pointer>(current_));
        }

        [[nodiscard]] reference operator*() {
            return *std::launder(reinterpret_cast<pointer>(current_));
        }

        [[nodiscard]] pointer operator->() const {
            return std::launder(reinterpret_cast<pointer>(current_));
        }

        [[nodiscard]] reference operator*() const {
            return *std::launder(reinterpret_cast<pointer>(current_));
        }

        [[nodiscard]] friend difference_type operator-(const BasicStridingIterator& a, const BasicStridingIterator& b) {
            return (a.current_ - b.current_) / a.stride_;
        }

        [[nodiscard]] bool operator==(const BasicStridingIterator&) const = default;

        [[nodiscard]] auto operator<=>(const BasicStridingIterator& other) const {
            return current_ <=> other.current_;
        }

        BasicStridingIterator& operator+=(difference_type n) {
            current_ += n * stride_;

            return *this;
        }

        BasicStridingIterator& operator-=(difference_type n) {
            current_ -= n * stride_;

            return *this;
        }

        [[nodiscard]] reference operator[](difference_type n) const {
            return *(current_ + n * stride_);
        }

        [[nodiscard]] friend BasicStridingIterator operator+(BasicStridingIterator it, difference_type n) {
            return it += n;
        }

        [[nodiscard]] friend BasicStridingIterator operator+(difference_type n, BasicStridingIterator it) {
            return it += n;
        }

        [[nodiscard]] friend BasicStridingIterator operator-(BasicStridingIterator it, difference_type n) {
            return it -= n;
        }

        BasicStridingIterator& operator++() {
            current_ += stride_;

            return *this;
        }

        BasicStridingIterator operator++(int) {
            BasicStridingIterator copy = *this;

            ++(*this);

            return copy;
        }

        BasicStridingIterator& operator--() {
            current_ -= stride_;

            return *this;
        }

        BasicStridingIterator operator--(int) {
            BasicStridingIterator copy = *this;

            --(*this);

            return copy;
        }

    private:
        std::byte* current_;
        difference_type stride_;
    };

    template <typename T>
    using StridingIterator = BasicStridingIterator<T, false>;

    template <typename T>
    using ConstStridingIterator = BasicStridingIterator<T, true>;
}