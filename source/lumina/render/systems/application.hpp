#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <string>

namespace lumina::render {
    /**
     * @brief Represents a major.minor.patch-formatted version.
     *
     */
    struct Version {
        std::uint32_t major;
        std::uint32_t minor;
        std::uint32_t patch;

        [[nodiscard]] bool operator==(const Version& other) const {
            return major == other.major && minor == other.minor && patch == other.patch;
        }

        [[nodiscard]] bool operator!=(const Version& other) const {
            return !(*this == other);
        }
    };

    /**
     * @brief A configuration for an application.
     *
     */
    struct ApplicationConfig {
        std::string name;
        Version version;
    };

    /**
     * @brief Represents a single connection to the Operating System.
     *
     * Only provides OS integration for the current thread. Other threads are
     * required to have their own application instances to interface with the
     * OS.
     */
    class Application {
    public:
        Application() = default;

        /**
         * @brief Construct a new application.
         *
         * @param config The application configuration to use.
         */
        Application(const ApplicationConfig& config)
            : init_(true), name_(config.name), version_(config.version) {
            glfwInit();

            // disable automatic OpenGL contexts for windows
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        /**
         * @brief Destroy the application.
         *
         * Once this is called, this application can no longer be used to interact
         * with the OS, unless recreated.
         */
        ~Application() {
            destroy();
        }

        /**
         * @brief Constructs the application.
         *
         * @param config The application configuration to use.
         */
        void create(const ApplicationConfig& config) {
            if (init_) {
                return;
            }

            init_ = true;
            name_ = config.name;
            version_ = config.version;

            glfwInit();

            // disable automatic OpenGL contexts for windows
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        /**
         * @brief Destroys the application.
         *
         * Once this is called, this application can no longer be used to interact
         * with the OS, unless recreated.
         */
        void destroy() {
            if (!init_) {
                return;
            }

            glfwTerminate();

            init_ = false;
            name_ = "";
            version_ = {};
        }

        Application(const Application&) = delete;
        Application(Application&&) noexcept = default;

        Application& operator=(const Application&) = delete;
        Application& operator=(Application&&) noexcept = default;

        /**
         * @brief Provides the application's version.
         *
         * @return Version The version of the application.
         */
        [[nodiscard]] Version version() const {
            return version_;
        }

        /**
         * @brief Provides the application's name.
         *
         * @return std::string_view The name of the application.
         */
        [[nodiscard]] std::string_view name() const {
            return name_;
        }

        /**
         * @brief Polls Operating System events immediately.
         *
         * This is non-blocking; it will not wait for events.
         */
        void pollEvents() const {
            glfwPollEvents();
        }

        /**
         * @brief Waits for Operating System events.
         *
         * This is blocking; it will wait for events.
         */
        void waitForEvents() const {
            glfwWaitEvents();
        }

        /**
         * @brief Waits for Operating System events given a timeout.
         *
         * This is blocking; it will wait for events unless the timeout is reached.
         */
        void waitForEvents(double timeout) const {
            glfwWaitEventsTimeout(timeout);
        }

        /**
         * @brief Provides whether this application is in a usable state.
         *
         * @return true If the application is valid.
         * @return false If the appliaction is invalid.
         */
        [[nodiscard]] bool valid() const noexcept {
            return init_;
        }

        /**
         * @brief Provides whether this application is in a usable state.
         *
         * @return true If the application is valid.
         * @return false If the appliaction is invalid.
         */
        [[nodiscard]] explicit operator bool() const noexcept {
            return init_;
        }

    private:
        bool init_ = false;
        std::string name_;
        Version version_;
    };
}