#pragma once

#include <cstdint>

namespace lumina::ecs {
    class Entity final {
        using FullType = std::uint64_t;

    public:
        using ValueType = std::uint32_t;

        Entity() = default;
        ~Entity() = default;

        Entity(ValueType id, ValueType version);

        Entity(const Entity&) = default;
        Entity(Entity&&) noexcept = default;

        Entity& operator=(const Entity&) = default;
        Entity& operator=(Entity&&) noexcept = default;

        [[nodiscard]] ValueType id() const;
        [[nodiscard]] ValueType version() const;
        [[nodiscard]] bool alive() const;

        [[nodiscard]] bool operator==(const Entity& other) const;
        [[nodiscard]] bool operator!=(const Entity& other) const;

        explicit operator bool() const;

    private:
        FullType state_ = 0xFFFFFFFF00000000ull;
    };
}
