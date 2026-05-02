#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <cstdint>
#include <lumina/core/application.hpp>
#include <string_view>
#include <vulkan/vulkan.h>

namespace lumina::render {
    struct Extent2D {
        std::uint32_t width;
        std::uint32_t height;
    };

    struct WindowConfig {
        std::string_view title;
        bool resizable;
        Extent2D extent;
    };

    class Window {
    public:
        Window(Application&, const WindowConfig& config)
            : title_(config.title), extent_(config.extent) {
            glfwWindowHint(GLFW_RESIZABLE, config.resizable);

            window_ = glfwCreateWindow(
                static_cast<int>(extent_.width),
                static_cast<int>(extent_.height),
                title_.c_str(),
                nullptr,
                nullptr);

            if (!window_) {
                std::printf("error: failed to create window\n");
                std::exit(1);
            }

            glfwSetWindowUserPointer(window_, this);
            glfwSetWindowSizeCallback(window_, onWindowResize);
            glfwSetWindowCloseCallback(window_, onWindowClose);
        }

        ~Window() {
            if (window_) {
                glfwDestroyWindow(window_);
                window_ = nullptr;
            }
        }

        [[nodiscard]] bool alive() const {
            return !closeSignalled_;
        }

        [[nodiscard]] Extent2D extent() const {
            return extent_;
        }

        [[nodiscard]] GLFWwindow* handle() const {
            return window_;
        }

    private:
        std::string title_;
        Extent2D extent_;
        GLFWwindow* window_ = nullptr;

        bool closeSignalled_ = false;

        static void onWindowResize(GLFWwindow* window, int width, int height) {
            void* userPointer = glfwGetWindowUserPointer(window);

            if (userPointer == nullptr) {
                return;
            }

            Window& target = *reinterpret_cast<Window*>(userPointer);

            target.extent_ = {
                .width = static_cast<std::uint32_t>(width),
                .height = static_cast<std::uint32_t>(height),
            };
        }

        static void onWindowClose(GLFWwindow* window) {
            void* userPointer = glfwGetWindowUserPointer(window);

            if (userPointer == nullptr) {
                return;
            }

            Window& target = *reinterpret_cast<Window*>(userPointer);

            target.closeSignalled_ = true;
        }
    };
}