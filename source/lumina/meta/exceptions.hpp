#pragma once

#include <format>
#include <stdexcept>

namespace lumina::meta {
    class Exception : public std::exception {
    public:
        template <typename... Args>
        Exception(std::format_string<Args...> message, Args&&... args)
            : message_(std::format(message, std::forward<Args>(args)...)) {
        }

        ~Exception() override = default;

        [[nodiscard]] const char* what() const noexcept override {
            return message_.c_str();
        }

    private:
        std::string message_;
    };
}