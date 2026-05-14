#pragma once

#include "console.hpp"

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

    /**
     * @brief Ensures a condition is true, otherwise it throws an exception
     *
     * @tparam Args The format argument types.
     * @param condition The condition to check.
     * @param message The message format string.
     * @param args The format arguments.
     */
    template <typename... Args>
    inline void assert(bool condition, std::format_string<Args...> message, Args&&... args) {
        if (!condition) {
            throw Exception(message, std::forward<Args>(args)...);
        }
    }

    /**
     * @brief Ensures a condition is true, otherwise it terminates the program
     *
     * @tparam Args The format argument types.
     * @param condition The condition to check.
     * @param message The message format string.
     * @param args The format arguments.
     */
    template <typename... Args>
    inline void cassert(bool condition, std::format_string<Args...> message, Args&&... args) {
        if (!condition) {
            meta::logError(message, std::forward<Args>(args)...);
            std::exit(1);
        }
    }

    /**
     * @brief Logs some error information, then terminates the process.
     *
     * @tparam Args The format argument types.
     * @param message The message format string.
     * @param args The format arguments.
     */
    template <typename... Args>
    inline void terminate(std::format_string<Args...> message, Args&&... args) {
        logError(message, std::forward<Args>(args)...);

        std::exit(1);
    }
}
