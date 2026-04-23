#pragma once

#include <cstdint>

namespace vesta {
    class entity final {
        using full_type = std::uint64_t;

    public:
        using value_type = std::uint32_t;

        entity() = default;

        entity(value_type id, value_type version) {
            state_ = (state_ & 0x00000000FFFFFFFFull) | static_cast<full_type>(id) << 32;
            state_ = (state_ & 0xFFFFFFFF00000000ull) | static_cast<full_type>(version);
        }

        ~entity() = default;

        entity(const entity&) = default;
        entity(entity&&) noexcept = default;

        entity& operator=(const entity&) = default;
        entity& operator=(entity&&) noexcept = default;

        [[nodiscard]] value_type id() const {
            return static_cast<value_type>(state_ >> 32);
        }

        [[nodiscard]] value_type version() const {
            return static_cast<value_type>(state_);
        }

        [[nodiscard]] bool alive() const {
            return id() != 0xFFFFFFFFu;
        }

        [[nodiscard]] bool operator==(const entity& other) const {
            return state_ == other.state_;
        }

        [[nodiscard]] bool operator!=(const entity& other) const {
            return state_ != other.state_;
        }

        explicit operator bool() const {
            return alive();
        }

    private:
        full_type state_ = 0xFFFFFFFF00000000ull;
    };
}
