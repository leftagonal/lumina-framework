#pragma once

#include <cstddef>

namespace lumina::renderer {
    template <typename T>
    class Handle final {
    public:
        using TagType = T;

        Handle() = default;

        Handle(std::size_t id)
            : id_(id) {
        }

        ~Handle() = default;

        Handle(const Handle&) = default;
        Handle(Handle&&) noexcept = default;

        Handle& operator=(const Handle&) = default;
        Handle& operator=(Handle&&) noexcept = default;

        [[nodiscard]] std::size_t id() const {
            return id_;
        }

        [[nodiscard]] bool valid() const {
            return id() != 0xFFFFFFFFFFFFFFFFull;
        }

        [[nodiscard]] bool operator==(const Handle& other) const {
            return id_ == other.id_;
        }

        [[nodiscard]] bool operator!=(const Handle& other) const {
            return id_ != other.id_;
        }

        explicit operator bool() const {
            return valid();
        }

    private:
        std::size_t id_ = 0xFFFFFFFFFFFFFFFFull;
    };
}
