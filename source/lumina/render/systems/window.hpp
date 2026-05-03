#pragma once

#include "../meta/resource.hpp"
#include "application.hpp"

#include <cstdint>
#include <string_view>

namespace lumina::render {
    /**
     * @brief A 2-dimensional bounding box or rectangle.
     *
     */
    struct Extent2D {
        std::uint32_t width;
        std::uint32_t height;
    };

    /**
     * @brief The configuration for a window.
     *
     */
    struct WindowConfig {
        std::string_view title;
        bool resizable;
        Extent2D extent;
    };

    /**
     * @brief A surface or "window" presented on-screen by the Operating System.
     *
     */
    class Window : public Resource<GLFWwindow*> {
    public:
        Window() = default;

        /**
         * @brief Construct a new window.
         *
         * @param application The application to create the window under.
         * @param config The window configuration to use.
         */
        Window(Application& application, const WindowConfig& config)
            : application_(&application), title_(config.title), extent_(config.extent) {
            glfwWindowHint(GLFW_RESIZABLE, config.resizable);

            resource() = glfwCreateWindow(
                static_cast<int>(extent_.width),
                static_cast<int>(extent_.height),
                title_.c_str(),
                nullptr,
                nullptr);

            if (!*this) {
                std::printf("error: failed to create window\n");
                std::exit(1);
            }

            glfwSetWindowUserPointer(resource(), this);
            glfwSetFramebufferSizeCallback(resource(), onWindowResize);
            glfwSetWindowCloseCallback(resource(), onWindowClose);
        }

        /**
         * @brief Destroy the window.
         *
         */
        ~Window() {
            destroy();
        }

        Window(Window&&) noexcept = default;

        Window& operator=(Window&&) noexcept = default;

        /**
         * @brief Construct the window.
         *
         * @param application The application to create the window under.
         * @param config The window configuration to use.
         */
        void create(Application& application, const WindowConfig& config) {
            if (*this) {
                return;
            }

            application_ = &application;
            title_ = config.title;
            extent_ = config.extent;

            glfwWindowHint(GLFW_RESIZABLE, config.resizable);

            resource() = glfwCreateWindow(
                static_cast<int>(extent_.width),
                static_cast<int>(extent_.height),
                title_.c_str(),
                nullptr,
                nullptr);

            if (!*this) {
                std::printf("error: failed to create window\n");
                std::exit(1);
            }

            glfwSetWindowUserPointer(resource(), this);
            glfwSetWindowCloseCallback(resource(), onWindowClose);
            glfwSetWindowSizeCallback(resource(), onWindowResize);
        }

        /**
         * @brief Destroy the window.
         *
         */
        void destroy() {
            if (!*this) {
                return;
            }

            glfwDestroyWindow(resource());

            application_ = nullptr;

            invalidate();
        }

        /**
         * @brief Indicate whether the window has been instructed to close.
         *
         * @return true If the window must close.
         * @return false If the window can proceed.
         */
        [[nodiscard]] bool mustClose() const {
            return mustClose_;
        }

        /**
         * @brief Provides the current extent of the window.
         *
         * @return Extent2D The extent of the window.
         */
        [[nodiscard]] Extent2D extent() const {
            return extent_;
        }

        /**
         * @brief Provides the current title of the window.
         *
         * @return std::string_view The title of the window.
         */
        [[nodiscard]] std::string_view title() const {
            return title_;
        }

        /**
         * @brief Sets the extent of the window.
         *
         * This will fail if the window is not resizable.
         */
        void setExtent(const Extent2D& extent) {
            extent_ = extent;

            glfwSetWindowSize(
                resource(),
                static_cast<int>(extent_.width),
                static_cast<int>(extent_.height));
        }

        /**
         * @brief Sets the title of the window.
         *
         */
        void setTitle(std::string_view title) {
            title_ = title;

            glfwSetWindowTitle(resource(), title_.c_str());
        }

        /**
         * @brief Provides the application that this window operates under.
         *
         * @return Application& The window's application.
         */
        [[nodiscard]] Application& application() {
            return *application_;
        }

        /**
         * @brief Provides the application that this window operates under.
         *
         * @return const Application& The window's application.
         */
        [[nodiscard]] const Application& application() const {
            return *application_;
        }

    private:
        Application* application_ = nullptr;
        std::string title_;
        Extent2D extent_;
        bool mustClose_ = false;

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

            target.mustClose_ = true;
        }
    };
}