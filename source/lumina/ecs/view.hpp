#pragma once

#include <lumina/ecs/iterator.hpp>

namespace lumina::ecs {
    template <bool Const, traits::PODType... Ts>
    class BasicView final {
        using IndexType = Entity::ValueType;
        using IndexTable = structs::IndexTable<IndexType>;
        using IndexTables = const std::vector<IndexTable>;
        using Versions = const std::vector<IndexType>;
        using Allocations = std::conditional_t<Const, const std::vector<structs::Allocation>, std::vector<structs::Allocation>>;

    public:
        BasicView(IndexTables* tables, Allocations* allocations, Versions* versions)
            : tables_(tables), allocations_(allocations), versions_(versions) {
        }

        class Iterable final {
        public:
            Iterable(IndexTables* tables, Allocations* allocations, Versions* versions, std::size_t driver_index)
                : tables_(tables), allocations_(allocations), versions_(versions), driverIndex_(driver_index) {
            }

            [[nodiscard]] ScanningComponentIterator<Const, Ts...> begin() requires(!Const)
            {
                return ScanningComponentIterator<Const, Ts...>{
                    tables_,
                    allocations_,
                    versions_,
                    driverIndex_,
                    0,
                    maximum(),
                };
            }

            [[nodiscard]] ScanningComponentIterator<Const, Ts...> end() requires(!Const)
            {
                return ScanningComponentIterator<Const, Ts...>{
                    tables_,
                    allocations_,
                    versions_,
                    driverIndex_,
                    maximum(),
                    maximum(),
                };
            }

            [[nodiscard]] ScanningComponentIterator<Const, Ts...> begin() const requires(Const)
            {
                return ScanningComponentIterator<Const, Ts...>{
                    tables_,
                    allocations_,
                    versions_,
                    driverIndex_,
                    0,
                    maximum(),
                };
            }

            [[nodiscard]] ScanningComponentIterator<Const, Ts...> end() const requires(Const)
            {
                return ScanningComponentIterator<Const, Ts...>{
                    tables_,
                    allocations_,
                    versions_,
                    driverIndex_,
                    maximum(),
                    maximum(),
                };
            }

        private:
            IndexTables* tables_;
            Allocations* allocations_;
            Versions* versions_;
            std::size_t driverIndex_;

            [[nodiscard]] std::size_t maximum() const {
                return (*tables_)[driverIndex_].dense().size();
            }
        };

        class ExpandedIterable final {
        public:
            ExpandedIterable(IndexTables* tables, Allocations* allocations, Versions* versions, std::size_t driver_index)
                : tables_(tables), allocations_(allocations), versions_(versions), driverIndex_(driver_index) {
            }

            [[nodiscard]] ScanningExpandingComponentIterator<Const, Ts...> begin() requires(!Const)
            {
                return ScanningExpandingComponentIterator<Const, Ts...>{
                    tables_,
                    allocations_,
                    versions_,
                    driverIndex_,
                    0,
                    maximum(),
                };
            }

            [[nodiscard]] ScanningExpandingComponentIterator<Const, Ts...> end() requires(!Const)
            {
                return ScanningExpandingComponentIterator<Const, Ts...>{
                    tables_,
                    allocations_,
                    versions_,
                    driverIndex_,
                    maximum(),
                    maximum(),
                };
            }

            [[nodiscard]] ScanningExpandingComponentIterator<Const, Ts...> begin() const requires(Const)
            {
                return ScanningExpandingComponentIterator<Const, Ts...>{
                    tables_,
                    allocations_,
                    versions_,
                    driverIndex_,
                    0,
                    maximum(),
                };
            }

            [[nodiscard]] ScanningExpandingComponentIterator<Const, Ts...> end() const requires(Const)
            {
                return ScanningExpandingComponentIterator<Const, Ts...>{
                    tables_,
                    allocations_,
                    versions_,
                    driverIndex_,
                    maximum(),
                    maximum(),
                };
            }

        private:
            IndexTables* tables_;
            Allocations* allocations_;
            Versions* versions_;
            std::size_t driverIndex_;

            [[nodiscard]] std::size_t maximum() const {
                return (*tables_)[driverIndex_].dense().size();
            }
        };

        [[nodiscard]] Iterable immediate() {
            return Iterable{
                tables_,
                allocations_,
                versions_,
                getDriverIndex(),
            };
        }

        [[nodiscard]] ExpandedIterable expanded() {
            return ExpandedIterable{
                tables_,
                allocations_,
                versions_,
                getDriverIndex(),
            };
        }

        [[nodiscard]] Iterable immediate() const {
            return Iterable{
                tables_,
                allocations_,
                versions_,
                getDriverIndex(),
            };
        }

        [[nodiscard]] ExpandedIterable expanded() const {
            return ExpandedIterable{
                tables_,
                allocations_,
                versions_,
                getDriverIndex(),
            };
        }

    private:
        IndexTables* tables_;
        Allocations* allocations_;
        Versions* versions_;

        [[nodiscard]] std::size_t getDriverIndex() {
            std::size_t current = 0xFFFFFFFFFFFFFFFFull;

            (..., smallestOf<Ts>(current));

            return current;
        }

        template <traits::PODType T>
        void smallestOf(std::size_t& current) {
            std::size_t index = traits::typeIndex<traits::ECSTypeContext, T>();
            std::size_t size = (*tables_)[index].dense().size();

            current = std::min(current, size);
        }
    };

    template <traits::PODType... Ts>
    using View = BasicView<false, Ts...>;

    template <traits::PODType... Ts>
    using ConstView = BasicView<true, Ts...>;
}