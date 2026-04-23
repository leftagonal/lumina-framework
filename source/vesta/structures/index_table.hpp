#pragma once

#include <vector>

namespace vesta::internal {
    template <std::unsigned_integral T>
    class index_table final {
    public:
        using index_type = T;

        index_table() = default;
        ~index_table() = default;

        index_table(const index_table&) = default;
        index_table(index_table&&) noexcept = default;

        index_table& operator=(const index_table&) = default;
        index_table& operator=(index_table&&) = default;

        [[nodiscard]] bool operator==(const index_table& other) const {
            return dense_ == other.dense_ && sparse_ == other.sparse_;
        }

        [[nodiscard]] bool operator!=(const index_table& other) const {
            return !(*this == other);
        }

        [[nodiscard]] index_type operator[](index_type sparse) const {
            return sparse_[sparse];
        }

        void insert(index_type sparse) {
            if (contains(sparse)) {
                return;
            }

            if (sparse_.size() <= sparse) {
                sparse_.resize(std::max(static_cast<std::size_t>(sparse) + 1, sparse_.size() * 2), empty_sparse);
            }

            auto sparse_value = static_cast<index_type>(dense_.size());

            dense_.emplace_back(sparse);
            sparse_[sparse] = sparse_value;
        }

        [[nodiscard]] bool contains(index_type sparse) const {
            return sparse_.size() > sparse && sparse_[sparse] != empty_sparse;
        }

        void move_to_end(index_type sparse) {
            index_type dense = sparse_[sparse];
            index_type moved_sparse = dense_.back();

            dense_[dense] = moved_sparse;
            sparse_[moved_sparse] = dense;

            dense_.back() = sparse;
            sparse_[sparse] = dense_.size() - 1;
        }

        void pop() {
            index_type sparse = dense_.back();

            sparse_[sparse] = empty_sparse;
            dense_.pop_back();
        }

        void remove(index_type sparse) {
            if (!contains(sparse)) {
                return;
            }

            if (sparse != dense_.back()) {
                move_to_end(sparse);
            }

            pop();
        }

        [[nodiscard]] const std::vector<index_type>& dense() const {
            return dense_;
        }

        [[nodiscard]] const std::vector<index_type>& sparse() const {
            return sparse_;
        }

    private:
        static constexpr index_type empty_sparse = 0xFFFFFFFFu;

        std::vector<index_type> dense_;
        std::vector<index_type> sparse_;
    };
}
