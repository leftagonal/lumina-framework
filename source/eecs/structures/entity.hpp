#pragma once

#include <cstdint>

namespace eecs {
    class entity final {
    public:
        entity() = default;

        entity(std::uint32_t id, std::uint32_t version) {
            write_id(id);
            write_version(version);
        }

        ~entity() = default;

        entity(const entity&) = default;
        entity(entity&&) noexcept = default;

        entity& operator=(const entity&) = default;
        entity& operator=(entity&&) noexcept = default;

        [[nodiscard]] std::uint32_t id() const {
            return read_id();
        }

        [[nodiscard]] std::uint32_t version() const {
            return read_version();
        }

        [[nodiscard]] bool operator==(const entity& other) const {
            return state_ == other.state_;
        }

        [[nodiscard]] bool operator!=(const entity& other) const {
            return state_ != other.state_;
        }

        [[nodiscard]] bool valid() const {
            return read_id() != 0xFFFFFFFFu;
        }

        explicit operator bool() const {
            return valid();
        }

    private:
        std::uint64_t state_ = 0xFFFFFFFF00000000ull;

        [[nodiscard]] std::uint32_t read_id() const {
            return static_cast<std::uint32_t>(state_ >> 32);
        }

        [[nodiscard]] std::uint32_t read_version() const {
            return static_cast<std::uint32_t>(state_);
        }

        void write_id(std::uint32_t id) {
            state_ = (state_ & 0x00000000FFFFFFFFull) | static_cast<std::uint64_t>(id) << 32;
        }

        void write_version(std::uint32_t version) {
            state_ = (state_ & 0xFFFFFFFF00000000ull) | static_cast<std::uint64_t>(version);
        }
    };
}
