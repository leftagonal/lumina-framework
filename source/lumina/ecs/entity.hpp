#pragma once

#include <cstdint>

namespace lumina::ecs {
    class Entity final {
        using FullType = std::uint64_t;

    public:
        using ValueType = std::uint32_t;

        Entity() = default;

        Entity(ValueType id, ValueType version) {
            state_ = (state_ & 0x00000000FFFFFFFFull) | static_cast<FullType>(id) << 32;
            state_ = (state_ & 0xFFFFFFFF00000000ull) | static_cast<FullType>(version);
        }

        ~Entity() = default;

        Entity(const Entity&) = default;
        Entity(Entity&&) noexcept = default;

        Entity& operator=(const Entity&) = default;
        Entity& operator=(Entity&&) noexcept = default;

        [[nodiscard]] ValueType id() const {
            return static_cast<ValueType>(state_ >> 32);
        }

        [[nodiscard]] ValueType version() const {
            return static_cast<ValueType>(state_);
        }

        [[nodiscard]] bool alive() const {
            return id() != 0xFFFFFFFFu;
        }

        [[nodiscard]] bool operator==(const Entity& other) const {
            return state_ == other.state_;
        }

        [[nodiscard]] bool operator!=(const Entity& other) const {
            return state_ != other.state_;
        }

        explicit operator bool() const {
            return alive();
        }

    private:
        FullType state_ = 0xFFFFFFFF00000000ull;
    };
}
