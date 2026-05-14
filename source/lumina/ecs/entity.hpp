#pragma once

#include <cstddef>

namespace lumina::ecs {
    class Entity final {
    public:
        Entity() = default;
        ~Entity() = default;

        Entity(std::size_t id, std::size_t version);

        Entity(const Entity&) = default;
        Entity(Entity&&) noexcept = default;

        Entity& operator=(const Entity&) = default;
        Entity& operator=(Entity&&) noexcept = default;

        [[nodiscard]] std::size_t id() const;
        [[nodiscard]] std::size_t version() const;
        [[nodiscard]] bool alive() const;

        [[nodiscard]] bool operator==(const Entity& other) const;
        [[nodiscard]] bool operator!=(const Entity& other) const;

        explicit operator bool() const;

    private:
        std::size_t id_;
        std::size_t version_;
    };
}
