#pragma once

#include <vesta/traits/pod_type.hpp>
#include <vesta/traits/type_index.hpp>

#include <vesta/ecs/entity.hpp>
#include <vesta/ecs/set_functions.hpp>

#include <vesta/structures/allocation.hpp>
#include <vesta/structures/index_table.hpp>

#include <cstddef>
#include <iterator>

namespace vesta::internal {
    template <bool Const, bool Scanning, bool Expand, pod_type... Ts>
    class basic_component_iterator final {
        template <bool>
        struct expand_rule;

        template <>
        struct expand_rule<false> {
            using type = entity;
            using instance_type = std::conditional_t<Const, const basic_component_iterator, basic_component_iterator>;

            static type get(instance_type& iterator) {
                auto& driver = (*iterator.tables_)[iterator.driver_index_];
                auto index = static_cast<entity::value_type>(iterator.index_);
                auto id = driver.dense()[index];

                return entity{
                    id,
                    (*iterator.versions_)[id],
                };
            }
        };

        template <>
        struct expand_rule<true> {
            using type = std::conditional_t<Const, const std::tuple<entity, const Ts&...>, std::tuple<entity, Ts&...>>;
            using instance_type = std::conditional_t<Const, const basic_component_iterator, basic_component_iterator>;

            template <pod_type T>
            using component_type = std::conditional_t<Const, const T, T>;

            static type get(instance_type& iterator) {
                auto& driver = (*iterator.tables_)[iterator.driver_index_];
                auto index = static_cast<entity::value_type>(iterator.index_);
                auto id = driver.dense()[index];

                auto target = entity{
                    id,
                    (*iterator.versions_)[id],
                };

                return type{target, component_of<Ts>(id, iterator)...};
            }

            template <pod_type T>
            [[nodiscard]] static component_type<T>& component_of(entity::value_type id, instance_type& iterator) {
                std::size_t index = type_index<internal::ecs_type_context_t, T>();

                return set_functions::get<T>((*iterator.allocations_)[index], (*iterator.tables_)[index], id);
            }
        };

        using index_type = entity::value_type;
        using index_table = index_table<index_type>;
        using index_tables = const std::vector<index_table>;
        using versions = const std::vector<index_type>;
        using allocations = std::conditional_t<Const, const std::vector<allocation>, std::vector<allocation>>;

    public:
        using value_type = expand_rule<Expand>::type;
        using reference = value_type;
        using pointer = value_type;
        using difference_type = std::ptrdiff_t;

        basic_component_iterator(index_tables* tables, allocations* allocations, versions* versions, std::size_t driver_index, std::size_t index, std::size_t end)
            : tables_(tables), allocations_(allocations), versions_(versions), driver_index_(driver_index), index_(index), end_(end) {
        }

        ~basic_component_iterator() = default;

        basic_component_iterator(const basic_component_iterator&) = default;
        basic_component_iterator(basic_component_iterator&&) noexcept = default;

        basic_component_iterator& operator=(const basic_component_iterator&) = default;
        basic_component_iterator& operator=(basic_component_iterator&&) noexcept = default;

        [[nodiscard]] reference operator*() {
            return expand_rule<Expand>::get(*this);
        }

        [[nodiscard]] reference operator*() const {
            return expand_rule<Expand>::get(*this);
        }

        [[nodiscard]] friend difference_type operator-(const basic_component_iterator& a, const basic_component_iterator& b) {
            return a.index_ - b.index_;
        }

        [[nodiscard]] bool operator==(const basic_component_iterator&) const = default;

        [[nodiscard]] auto operator<=>(const basic_component_iterator& other) const {
            return index_ <=> other.index_;
        }

        basic_component_iterator& operator+=(difference_type n) {
            while (n != 0) {
                ++(*this);
                --n;
            }

            return *this;
        }

        basic_component_iterator& operator-=(difference_type n) {
            while (n != 0) {
                --(*this);
                --n;
            }

            return *this;
        }

        [[nodiscard]] friend basic_component_iterator operator+(basic_component_iterator it, difference_type n) {
            return it += n;
        }

        [[nodiscard]] friend basic_component_iterator operator+(difference_type n, basic_component_iterator it) {
            return it += n;
        }

        [[nodiscard]] friend basic_component_iterator operator-(basic_component_iterator it, difference_type n) {
            return it -= n;
        }

        basic_component_iterator& operator++() {
            ++index_;

            if constexpr (Scanning) {
                scan_forward();
            }

            return *this;
        }

        basic_component_iterator operator++(int) {
            basic_component_iterator copy = *this;

            ++(*this);

            return copy;
        }

        basic_component_iterator& operator--() {
            --index_;

            if constexpr (Scanning) {
                scan_backward();
            }

            return *this;
        }

        basic_component_iterator operator--(int) {
            basic_component_iterator copy = *this;

            --(*this);

            return copy;
        }

    private:
        index_tables* tables_;
        allocations* allocations_;
        versions* versions_;
        std::size_t driver_index_;
        std::size_t index_;
        std::size_t end_;

        void scan_forward() {
            auto& driver = (*tables_)[driver_index_];

            while (index_ != end_ && !has_all(driver.dense()[index_])) {
                ++index_;
            }
        }

        void scan_backward() {
            auto& driver = (*tables_)[driver_index_];

            while (index_ != 0 && !has_all(driver.dense()[index_])) {
                --index_;
            }
        }

        template <pod_type T>
        [[nodiscard]] bool has(entity::value_type id) {
            std::size_t index = type_index<internal::ecs_type_context_t, T>();

            return (*tables_)[index].contains(id);
        }

        [[nodiscard]] bool has_all(entity::value_type id) {
            return (... && has<Ts>(id));
        }
    };

    template <bool Const, pod_type... Ts>
    using component_iterator = basic_component_iterator<Const, false, false, Ts...>;

    template <bool Const, pod_type... Ts>
    using scanning_component_iterator = basic_component_iterator<Const, true, false, Ts...>;

    template <bool Const, pod_type... Ts>
    using expanding_component_iterator = basic_component_iterator<Const, false, true, Ts...>;

    template <bool Const, pod_type... Ts>
    using scanning_expanding_component_iterator = basic_component_iterator<Const, true, true, Ts...>;
}