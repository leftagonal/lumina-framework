#pragma once

#include <lumina/meta/exceptions.hpp>

#include <vector>
#include <cstddef>

namespace lumina::ecs {
    class IndexTable final {
    public:
        IndexTable() = default;
        ~IndexTable() = default;

        IndexTable(const IndexTable&) = default;
        IndexTable(IndexTable&&) noexcept = default;

        IndexTable& operator=(const IndexTable&) = default;
        IndexTable& operator=(IndexTable&&) = default;

        [[nodiscard]] bool operator==(const IndexTable& other) const {
            return dense_ == other.dense_ && sparse_ == other.sparse_;
        }

        [[nodiscard]] bool operator!=(const IndexTable& other) const {
            return !(*this == other);
        }

        [[nodiscard]] std::size_t& operator[](std::size_t sparse) {
            return sparse_[sparse];
        }

        [[nodiscard]] const std::size_t& operator[](std::size_t sparse) const {
            return sparse_[sparse];
        }

        [[nodiscard]] std::size_t& at(std::size_t sparse) {
            meta::assert(sparse < sparse_.size(), "index table out-of-range access (max: {} given: {})", sparse_.size() - 1, sparse);

            return sparse_[sparse];
        }

        [[nodiscard]] const std::size_t& at(std::size_t sparse) const {
            meta::assert(sparse < sparse_.size(), "index table out-of-range access (max: {} given: {})", sparse_.size() - 1, sparse);

            return sparse_[sparse];
        }

        void insert(std::size_t sparse) {
            if (contains(sparse)) {
                return;
            }

            if (sparse_.size() <= sparse) {
                sparse_.resize(std::max(static_cast<std::size_t>(sparse) + 1, sparse_.size() * 2), EmptySparse);
            }

            auto sparse_value = static_cast<std::size_t>(dense_.size());

            dense_.emplace_back(sparse);
            sparse_[sparse] = sparse_value;
        }

        [[nodiscard]] bool contains(std::size_t sparse) const {
            return sparse_.size() > sparse && sparse_[sparse] != EmptySparse;
        }

        void pop() {
            std::size_t sparse = dense_.back();

            sparse_[sparse] = EmptySparse;
            dense_.pop_back();
        }

        void moveBackTo(std::size_t sparse) {
            std::size_t back_sparse = dense_.back();
            std::size_t dense_index = sparse_[sparse];

            dense_[dense_index] = back_sparse;
            sparse_[back_sparse] = dense_index;
        }

        void remove(std::size_t sparse) {
            if (!contains(sparse)) {
                return;
            }

            if (sparse != dense_.back()) {
                moveBackTo(sparse);
            }

            pop();
        }

        void clear() {
            dense_.clear();
            sparse_.clear();
        }

        [[nodiscard]] const std::vector<std::size_t>& dense() const {
            return dense_;
        }

        [[nodiscard]] const std::vector<std::size_t>& sparse() const {
            return sparse_;
        }

    private:
        static constexpr std::size_t EmptySparse = 0xFFFFFFFFFFFFFFFFu;

        std::vector<std::size_t> dense_;
        std::vector<std::size_t> sparse_;
    };

    using IndexTables = std::vector<IndexTable>;
}
