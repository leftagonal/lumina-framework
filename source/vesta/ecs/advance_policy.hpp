#pragma once

#include <vesta/traits/pod_type.hpp>
#include <vesta/traits/type_index.hpp>

#include <vesta/structures/index_table.hpp>

#include <vesta/ecs/entity.hpp>

#include <span>

namespace vesta::internal {
    template <typename T>
    concept advance_policy = requires(
        std::span<const index_table<entity::value_type>> tables,
        std::size_t driver_index,
        std::size_t& index,
        std::size_t end) {
        { T::step_forward(tables, driver_index, index, end) } -> std::same_as<void>;
        { T::step_backward(tables, driver_index, index) } -> std::same_as<void>;
    };

    struct linear_advance_policy final {
        linear_advance_policy() = delete;

        static void step_forward(
            std::span<const index_table<entity::value_type>>,
            std::size_t,
            std::size_t& index,
            std::size_t end) {
            if (index != end) {
                ++index;
            }
        }

        static void step_backward(
            std::span<const index_table<entity::value_type>>,
            std::size_t,
            std::size_t& index) {
            if (index != 0) {
                --index;
            }
        }
    };

    template <pod_type... Ts>
    struct scanning_advance_policy final {
        scanning_advance_policy() = delete;

        static void step_forward(
            std::span<const index_table<entity::value_type>> tables,
            std::size_t driver_index,
            std::size_t& index,
            std::size_t end) {
            if (index != end) {
                ++index;
            }

            if (index == end) {
                return;
            }

            auto& driver = tables[driver_index];

            while (!has_all(driver.dense()[index], tables) && index != end) {
                ++index;
            }
        }

        static void step_backward(
            std::span<const index_table<entity::value_type>> tables,
            std::size_t driver_index,
            std::size_t& index) {
            if (index != 0) {
                --index;
            }

            if (index == 0) {
                return;
            }

            auto& driver = tables[driver_index];

            while (!has_all(driver.dense()[index], tables) && index != 0) {
                --index;
            }
        }

        template <pod_type T>
        [[nodiscard]] static bool has(
            std::uint32_t id,
            std::span<const index_table<entity::value_type>> tables) {
            std::size_t index = type_index<internal::ecs_type_context_t, T>();

            return tables[index].contains(id);
        }

        [[nodiscard]] static bool has_all(
            std::uint32_t id,
            std::span<const index_table<entity::value_type>> tables) {
            return (... && has<Ts>(id, tables));
        }
    };

    static_assert(
        advance_policy<linear_advance_policy>,
        "vesta::internal::linear_advance_policy must satisfy the advance_policy concept");

    static_assert(
        advance_policy<scanning_advance_policy<>>,
        "vesta::internal::scanning_advance_policy must satisfy the advance_policy concept");
}