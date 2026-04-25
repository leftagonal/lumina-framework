#pragma once

#include <vesta/traits/pod_type.hpp>
#include <vesta/traits/type_index.hpp>

#include <vesta/ecs/entity.hpp>
#include <vesta/ecs/set_functions.hpp>

#include <vesta/structures/allocation.hpp>
#include <vesta/structures/index_table.hpp>

#include <span>

namespace vesta::internal {
    template <typename T>
    concept not_void = !std::is_same_v<T, void>;

    template <bool Const, template <bool, typename...> typename T, typename... Ts>
    concept access_policy = requires(
        std::span<const index_table<entity::value_type>> tables,
        std::span<std::conditional_t<Const, const allocation, allocation>> allocations,
        std::span<const entity::value_type> versions,
        std::size_t driver_index,
        std::size_t index) {
        { T<Const>::get(tables, allocations, versions, driver_index, index) } -> not_void;
    };

    template <bool Const>
    struct packed_access_policy final {
        packed_access_policy() = delete;

        using type = entity;

        static type get(
            std::span<const index_table<entity::value_type>> tables,
            std::span<std::conditional_t<Const, const allocation, allocation>>,
            std::span<const entity::value_type> versions,
            std::size_t driver_index,
            std::size_t index) {
            auto& driver = tables[driver_index];

            entity::value_type id = driver.dense()[static_cast<std::uint32_t>(index)];

            return entity{
                id,
                versions[id],
            };
        }
    };

    template <bool Const, pod_type... Ts>
    struct unpacked_access_policy final {
        unpacked_access_policy() = delete;

        using type = std::tuple<entity, std::conditional_t<Const, const Ts, Ts>&...>;

        static type get(
            std::span<const index_table<entity::value_type>> tables,
            std::span<std::conditional_t<Const, const allocation, allocation>> allocations,
            std::span<const entity::value_type> versions,
            std::size_t driver_index,
            std::size_t index) {
            auto& driver = tables[driver_index];

            entity::value_type id = driver.dense()[static_cast<std::uint32_t>(index)];

            entity target = {
                id,
                versions[id],
            };

            return type{target, get<Ts>(id, tables, allocations)...};
        }

        template <pod_type T>
        [[nodiscard]] static std::conditional_t<Const, const T, T>& get(
            entity::value_type id,
            std::span<const index_table<entity::value_type>> tables,
            std::span<std::conditional_t<Const, const allocation, allocation>> allocations) {
            std::size_t index = type_index<internal::ecs_type_context_t, T>();

            set_functions::get<T>(allocations[index], tables[index], id);
        }
    };

    static_assert(
        access_policy<false, packed_access_policy> && access_policy<true, packed_access_policy>,
        "vesta::internal::packed_access_policy must satisfy the access_policy concept");

    static_assert(
        access_policy<false, unpacked_access_policy> && access_policy<false, unpacked_access_policy>,
        "vesta::internal::unpacked_access_policy must satisfy the access_policy concept");
}