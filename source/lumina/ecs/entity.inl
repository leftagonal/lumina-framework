#pragma once

#include "entity.hpp"

namespace lumina::ecs {
    inline Entity::Entity(ValueType id, ValueType version) {
        state_ = (state_ & 0x00000000FFFFFFFFull) | static_cast<FullType>(id) << 32;
        state_ = (state_ & 0xFFFFFFFF00000000ull) | static_cast<FullType>(version);
    }

    [[nodiscard]] inline Entity::ValueType Entity::id() const {
        return static_cast<ValueType>(state_ >> 32);
    }

    [[nodiscard]] inline Entity::ValueType Entity::version() const {
        return static_cast<ValueType>(state_);
    }

    [[nodiscard]] inline bool Entity::alive() const {
        return id() != 0xFFFFFFFFu;
    }

    [[nodiscard]] inline bool Entity::operator==(const Entity& other) const {
        return state_ == other.state_;
    }

    [[nodiscard]] inline bool Entity::operator!=(const Entity& other) const {
        return state_ != other.state_;
    }

    inline Entity::operator bool() const {
        return alive();
    }
}
