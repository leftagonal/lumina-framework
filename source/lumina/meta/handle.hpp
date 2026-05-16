#pragma once

#include <cstddef>

namespace lumina::meta {
    template <typename T>
    class Handle final {
    public:
        using Type = T;

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

        [[nodiscard]] bool operator==(const Handle& other) const {
            return id_ == other.id_;
        }

        [[nodiscard]] bool operator!=(const Handle& other) const {
            return id_ != other.id_;
        }

    private:
        std::size_t id_;
    };
}
