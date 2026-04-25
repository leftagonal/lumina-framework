#pragma once

#include <vesta/ecs/component_iterators.hpp>

namespace vesta::internal {
    template <bool Const, internal::pod_type... Ts>
    class basic_view final {
        using index_type = entity::value_type;
        using index_table = internal::index_table<index_type>;
        using index_tables = const std::vector<index_table>;
        using versions = const std::vector<index_type>;
        using allocations = std::conditional_t<Const, const std::vector<internal::allocation>, std::vector<internal::allocation>>;

    public:
        basic_view(index_tables* tables, allocations* allocations, versions* versions)
            : tables_(tables), allocations_(allocations), versions_(versions) {
        }

        class iterable final {
        public:
            iterable(index_tables* tables, allocations* allocations, versions* versions, std::size_t driver_index)
                : tables_(tables), allocations_(allocations), versions_(versions), driver_index_(driver_index) {
            }

            [[nodiscard]] internal::scanning_component_iterator<Const, Ts...> begin() requires(!Const)
            {
                return internal::scanning_component_iterator<Const, Ts...>{
                    tables_,
                    allocations_,
                    versions_,
                    driver_index_,
                    0,
                    maximum(),
                };
            }

            [[nodiscard]] internal::scanning_component_iterator<Const, Ts...> end() requires(!Const)
            {
                return internal::scanning_component_iterator<Const, Ts...>{
                    tables_,
                    allocations_,
                    versions_,
                    driver_index_,
                    maximum(),
                    maximum(),
                };
            }

            [[nodiscard]] internal::scanning_component_iterator<Const, Ts...> begin() const requires(Const)
            {
                return internal::scanning_component_iterator<Const, Ts...>{
                    tables_,
                    allocations_,
                    versions_,
                    driver_index_,
                    0,
                    maximum(),
                };
            }

            [[nodiscard]] internal::scanning_component_iterator<Const, Ts...> end() const requires(Const)
            {
                return internal::scanning_component_iterator<Const, Ts...>{
                    tables_,
                    allocations_,
                    versions_,
                    driver_index_,
                    maximum(),
                    maximum(),
                };
            }

        private:
            index_tables* tables_;
            allocations* allocations_;
            versions* versions_;
            std::size_t driver_index_;

            [[nodiscard]] std::size_t maximum() const {
                return (*tables_)[driver_index_].dense().size();
            }
        };

        class expanded_iterable final {
        public:
            expanded_iterable(index_tables* tables, allocations* allocations, versions* versions, std::size_t driver_index)
                : tables_(tables), allocations_(allocations), versions_(versions), driver_index_(driver_index) {
            }

            [[nodiscard]] internal::scanning_expanding_component_iterator<Const, Ts...> begin() requires(!Const)
            {
                return internal::scanning_expanding_component_iterator<Const, Ts...>{
                    tables_,
                    allocations_,
                    versions_,
                    driver_index_,
                    0,
                    maximum(),
                };
            }

            [[nodiscard]] internal::scanning_expanding_component_iterator<Const, Ts...> end() requires(!Const)
            {
                return internal::scanning_expanding_component_iterator<Const, Ts...>{
                    tables_,
                    allocations_,
                    versions_,
                    driver_index_,
                    maximum(),
                    maximum(),
                };
            }

            [[nodiscard]] internal::scanning_expanding_component_iterator<Const, Ts...> begin() const requires(Const)
            {
                return internal::scanning_expanding_component_iterator<Const, Ts...>{
                    tables_,
                    allocations_,
                    versions_,
                    driver_index_,
                    0,
                    maximum(),
                };
            }

            [[nodiscard]] internal::scanning_expanding_component_iterator<Const, Ts...> end() const requires(Const)
            {
                return internal::scanning_expanding_component_iterator<Const, Ts...>{
                    tables_,
                    allocations_,
                    versions_,
                    driver_index_,
                    maximum(),
                    maximum(),
                };
            }

        private:
            index_tables* tables_;
            allocations* allocations_;
            versions* versions_;
            std::size_t driver_index_;

            [[nodiscard]] std::size_t maximum() const {
                return (*tables_)[driver_index_].dense().size();
            }
        };

        [[nodiscard]] iterable immediate() {
            return iterable{
                tables_,
                allocations_,
                versions_,
                get_driver_index(),
            };
        }

        [[nodiscard]] expanded_iterable expanded() {
            return expanded_iterable{
                tables_,
                allocations_,
                versions_,
                get_driver_index(),
            };
        }

        [[nodiscard]] iterable immediate() const {
            return iterable{
                tables_,
                allocations_,
                versions_,
                get_driver_index(),
            };
        }

        [[nodiscard]] expanded_iterable expanded() const {
            return expanded_iterable{
                tables_,
                allocations_,
                versions_,
                get_driver_index(),
            };
        }

    private:
        index_tables* tables_;
        allocations* allocations_;
        versions* versions_;

        [[nodiscard]] std::size_t get_driver_index() {
            std::size_t current = 0xFFFFFFFFFFFFFFFFull;

            (..., smallest_of<Ts>(current));

            return current;
        }

        template <internal::pod_type T>
        void smallest_of(std::size_t& current) {
            std::size_t index = internal::type_index<internal::ecs_type_context_t, T>();
            std::size_t size = (*tables_)[index].dense().size();

            current = std::min(current, size);
        }
    };
}