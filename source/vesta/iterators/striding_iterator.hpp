#pragma once

#include <cstddef>
#include <iterator>

namespace vesta::internal {
    template <bool Const>
    class basic_striding_iterator final {
    public:
        using value_type = std::byte;
        using reference = std::conditional_t<Const, const value_type, value_type>&;
        using pointer = std::conditional_t<Const, const value_type, value_type>*;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;

        basic_striding_iterator(pointer current, difference_type stride)
            : current_(current), stride_(stride) {
        }

        ~basic_striding_iterator() = default;

        basic_striding_iterator(const basic_striding_iterator&) = default;
        basic_striding_iterator(basic_striding_iterator&&) noexcept = default;

        basic_striding_iterator& operator=(const basic_striding_iterator&) = default;
        basic_striding_iterator& operator=(basic_striding_iterator&&) noexcept = default;

        [[nodiscard]] pointer operator->() {
            return current_;
        }

        [[nodiscard]] reference operator*() {
            return *current_;
        }

        [[nodiscard]] pointer operator->() const {
            return current_;
        }

        [[nodiscard]] reference operator*() const {
            return *current_;
        }

        [[nodiscard]] friend difference_type operator-(const basic_striding_iterator& a, const basic_striding_iterator& b) {
            return (a.current_ - b.current_) / a.stride_;
        }

        [[nodiscard]] bool operator==(const basic_striding_iterator&) const = default;
        [[nodiscard]] auto operator<=>(const basic_striding_iterator& other) const {

            return current_ <=> other.current_;
        }

        basic_striding_iterator& operator+=(difference_type n) {
            current_ += n * stride_;
            return *this;
        }

        basic_striding_iterator& operator-=(difference_type n) {
            current_ -= n * stride_;
            return *this;
        }

        [[nodiscard]] reference operator[](difference_type n) const {
            return *(current_ + n * stride_);
        }

        [[nodiscard]] friend basic_striding_iterator operator+(basic_striding_iterator it, difference_type n) {
            return it += n;
        }

        [[nodiscard]] friend basic_striding_iterator operator+(difference_type n, basic_striding_iterator it) {
            return it += n;
        }

        [[nodiscard]] friend basic_striding_iterator operator-(basic_striding_iterator it, difference_type n) {
            return it -= n;
        }

        basic_striding_iterator& operator++() {
            current_ += stride_;

            return *this;
        }

        basic_striding_iterator operator++(int) {
            basic_striding_iterator copy = *this;

            ++(*this);

            return copy;
        }

        basic_striding_iterator& operator--() {
            current_ -= stride_;

            return *this;
        }

        basic_striding_iterator operator--(int) {
            basic_striding_iterator copy = *this;

            --(*this);

            return copy;
        }

    private:
        pointer current_;
        difference_type stride_;
    };

    using striding_iterator = basic_striding_iterator<false>;
    using const_striding_iterator = basic_striding_iterator<true>;
}