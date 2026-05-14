#pragma once

#include "entity.hpp"

namespace lumina::ecs {
    inline Entity::Entity(std::size_t id, std::size_t version)
        : id_(id), version_(version) {
    }

    [[nodiscard]] inline std::size_t Entity::id() const {
        return id_;
    }

    [[nodiscard]] inline std::size_t Entity::version() const {
        return version_;
    }

    [[nodiscard]] inline bool Entity::alive() const {
        return id_ != 0xFFFFFFFFFFFFFFFFull;
    }

    [[nodiscard]] inline bool Entity::operator==(const Entity& other) const {
        return id_ == other.id_ && version_ == other.version_;
    }

    [[nodiscard]] inline bool Entity::operator!=(const Entity& other) const {
        return id_ == other.id_ && version_ == other.version_;
    }

    inline Entity::operator bool() const {
        return alive();
    }
}
