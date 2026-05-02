#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <cstdint>
#include <string>

namespace lumina {
    struct Version {
        std::uint32_t major;
        std::uint32_t minor;
        std::uint32_t patch;
    };

    struct ApplicationConfig {
        std::string name;
        Version version;
    };

    class Application {
    public:
        Application(const ApplicationConfig& config)
            : name_(config.name), version_(config.version) {
            glfwInit();

            // disable automatic OpenGL contexts for windows
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        ~Application() {
            glfwTerminate();
        }

        [[nodiscard]] Version version() const {
            return version_;
        }

        [[nodiscard]] std::string_view name() const {
            return name_;
        }

        void pollEvents() {
            glfwPollEvents();
        }

        void waitForEvents() {
            glfwWaitEvents();
        }

        void waitForEvents(double timeout) {
            glfwWaitEventsTimeout(timeout);
        }

    private:
        std::string name_;
        Version version_;
    };
}