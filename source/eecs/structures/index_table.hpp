#pragma once

#include <cstdint>
#include <vector>

namespace eecs {
    class index_table final {
    public:
        index_table() = default;
        ~index_table() = default;

        index_table(const index_table&) = default;
        index_table(index_table&&) noexcept = default;

        index_table& operator=(const index_table&) = default;
        index_table& operator=(index_table&&) = default;

        using type = std::uint32_t;

        [[nodiscard]] bool operator==(const index_table& other) const {
            return dense_ == other.dense_ && sparse_ == other.sparse_;
        }

        [[nodiscard]] bool operator!=(const index_table& other) const {
            return !(*this == other);
        }

        [[nodiscard]] type operator[](type sparse) const {
            return sparse_[sparse];
        }

        void insert(type sparse) {
            if (contains(sparse)) {
                return;
            }

            if (sparse_.size() <= sparse) {
                sparse_.resize(sparse + 1, empty_sparse);
            }

            auto sparse_value = static_cast<type>(dense_.size());

            dense_.emplace_back(sparse);
            sparse_[sparse] = sparse_value;
        }

        [[nodiscard]] bool contains(type sparse) const {
            return sparse_.size() > sparse && sparse_[sparse] != empty_sparse;
        }

        void remove(type sparse) {
            if (!contains(sparse)) {
                return;
            }

            if (sparse != dense_.back()) {
                dense_[sparse_[sparse]] = dense_.back();
                sparse_[sparse] = sparse_[dense_.back()];
            }

            sparse_[sparse] = empty_sparse;
            dense_.pop_back();
        }

    private:
        std::vector<type> dense_;
        std::vector<type> sparse_;

        static constexpr type empty_sparse = 0xFFFFFFFFu;
    };
}
