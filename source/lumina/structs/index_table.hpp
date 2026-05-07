#pragma once

#include <vector>

namespace lumina::structs {
    template <std::unsigned_integral T>
    class IndexTable final {
    public:
        using IndexType = T;

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

        [[nodiscard]] IndexType operator[](IndexType sparse) const {
            return sparse_[sparse];
        }

        void insert(IndexType sparse) {
            if (contains(sparse)) {
                return;
            }

            if (sparse_.size() <= sparse) {
                sparse_.resize(std::max(static_cast<std::size_t>(sparse) + 1, sparse_.size() * 2), EmptySparse);
            }

            auto sparse_value = static_cast<IndexType>(dense_.size());

            dense_.emplace_back(sparse);
            sparse_[sparse] = sparse_value;
        }

        [[nodiscard]] bool contains(IndexType sparse) const {
            return sparse_.size() > sparse && sparse_[sparse] != EmptySparse;
        }

        void moveToEnd(IndexType sparse) {
            IndexType dense = sparse_[sparse];
            IndexType movedSparse = dense_.back();

            dense_[dense] = movedSparse;
            sparse_[movedSparse] = dense;

            dense_.back() = sparse;
            sparse_[sparse] = dense_.size() - 1;
        }

        void pop() {
            IndexType sparse = dense_.back();

            sparse_[sparse] = EmptySparse;
            dense_.pop_back();
        }

        void remove(IndexType sparse) {
            if (!contains(sparse)) {
                return;
            }

            if (sparse != dense_.back()) {
                moveToEnd(sparse);
            }

            pop();
        }

        void clear() {
            dense_.clear();
            sparse_.clear();
        }

        [[nodiscard]] const std::vector<IndexType>& dense() const {
            return dense_;
        }

        [[nodiscard]] const std::vector<IndexType>& sparse() const {
            return sparse_;
        }

    private:
        static constexpr IndexType EmptySparse = 0xFFFFFFFFu;

        std::vector<IndexType> dense_;
        std::vector<IndexType> sparse_;
    };
}
