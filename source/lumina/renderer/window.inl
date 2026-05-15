#pragma once

#include <lumina/meta/exceptions.hpp>

#include "instance.hpp"
#include "window.hpp"

namespace lumina::renderer {
    Window::Window(const WindowHandle& handle, Instance& instance, WindowInfo& info)
        : instance_(&instance), sizeStream_(info.sizeStream), statusStream_(info.statusStream),
          focusStream_(info.focusStream), handle_(handle) {
        glfwWindowHint(GLFW_RESIZABLE, info.features.resizable);

        state_.current.extent = info.extent;
        state_.current.title = info.title;

        int width = static_cast<int>(info.extent.width);
        int height = static_cast<int>(info.extent.height);

        window_ = glfwCreateWindow(width, height, info.title.data(), nullptr, nullptr);

        meta::assert(window_ != nullptr, "failed to create window");

        auto instanceHandle = accessors::InstanceAccessor::instance(*instance_);

        VkResult result = glfwCreateWindowSurface(instanceHandle, window_, nullptr, &surface_);

        meta::assert(result == VK_SUCCESS, "failed to create window surface: {}", Vulkan_errorString(result));

        glfwSetWindowUserPointer(window_, this);
        glfwSetFramebufferSizeCallback(window_, framebufferResizeCallback);
        glfwSetWindowFocusCallback(window_, focusCallback);
        glfwSetWindowIconifyCallback(window_, iconifyCallback);
        glfwSetWindowCloseCallback(window_, closeCallback);

        state_.current.status = WindowStatus::Active;
        state_.current.focus = WindowFocus::Focused;

        state_.previous = state_.current;
    }

    Window::~Window() {
        destroy();
    }

    Window::Window(Window&& other) noexcept
        : instance_(other.instance_), sizeStream_(other.sizeStream_), statusStream_(other.statusStream_),
          focusStream_(other.focusStream_), window_(other.window_), surface_(other.surface_), handle_(other.handle_), state_(other.state_) {
        glfwSetWindowUserPointer(window_, this);

        other.instance_ = nullptr;
        other.sizeStream_ = nullptr;
        other.statusStream_ = nullptr;
        other.focusStream_ = nullptr;
        other.window_ = nullptr;
        other.surface_ = nullptr;
    }

    Window& Window::operator=(Window&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        instance_ = other.instance_;
        sizeStream_ = other.sizeStream_;
        statusStream_ = other.statusStream_;
        focusStream_ = other.focusStream_;
        window_ = other.window_;
        surface_ = other.surface_;
        handle_ = other.handle_;
        state_ = other.state_;

        glfwSetWindowUserPointer(window_, this);

        other.instance_ = nullptr;
        other.window_ = nullptr;
        other.surface_ = nullptr;
        other.sizeStream_ = nullptr;
        other.statusStream_ = nullptr;
        other.focusStream_ = nullptr;

        return *this;
    }

    [[nodiscard]] auto Window::current() const -> WindowState {
        return state_.current;
    }

    [[nodiscard]] auto Window::previous() const -> WindowState {
        return state_.previous;
    }

    void Window::resize(const Extent2D& extent) {
        state_.previous = state_.current;
        state_.current.extent = extent;

        int width = static_cast<int>(extent.width);
        int height = static_cast<int>(extent.height);

        glfwSetWindowSize(window_, width, height);
    }

    void Window::rename(std::string_view title) {
        state_.previous = state_.current;
        state_.current.title = title;

        glfwSetWindowTitle(window_, title.data());
    }

    void Window::resurface() {
        auto instanceHandle = accessors::InstanceAccessor::instance(*instance_);

        if (surface_) {
            vkDestroySurfaceKHR(instanceHandle, surface_, nullptr);
        }

        VkResult result = glfwCreateWindowSurface(instanceHandle, window_, nullptr, &surface_);

        meta::assert(result == VK_SUCCESS, "failed to create window surface: {}", Vulkan_errorString(result));
    }

    void Window::destroy() {
        if (window_) {
            glfwDestroyWindow(window_);

            window_ = nullptr;
        }

        if (surface_) {
            auto instanceHandle = accessors::InstanceAccessor::instance(*instance_);

            vkDestroySurfaceKHR(instanceHandle, surface_, nullptr);

            surface_ = nullptr;
            instance_ = nullptr;
        }

        sizeStream_ = nullptr;
        statusStream_ = nullptr;
        focusStream_ = nullptr;
    }

    bool Window::valid() const {
        return window_ != nullptr && surface_ != nullptr;
    }

    Window::operator bool() const {
        return valid();
    }

    Window& Window::extract(GLFWwindow* handle) {
        void* userPointer = glfwGetWindowUserPointer(handle);

        meta::cassert(userPointer != nullptr, "window lost");

        return *static_cast<Window*>(userPointer);
    }

    void Window::framebufferResizeCallback(GLFWwindow* handle, int width, int height) {
        Window& window = extract(handle);

        if (window.state_.current.status == WindowStatus::Inactive) {
            return;
        }

        window.state_.previous = window.state_.current;

        window.state_.current.extent = {
            static_cast<std::size_t>(width),
            static_cast<std::size_t>(height),
        };

        if (!window.sizeStream_) {
            return;
        }

        WindowResize event = {
            .target = window.handle_,
            .oldExtent = window.state_.previous.extent,
            .newExtent = window.state_.current.extent,
        };

        window.sizeStream_->enqueue(event);
    }

    void Window::focusCallback(GLFWwindow* handle, int status) {
        Window& window = extract(handle);

        if (window.state_.current.status == WindowStatus::Inactive) {
            return;
        }

        window.state_.previous = window.state_.current;

        if (status) {
            window.state_.current.focus = WindowFocus::Focused;
        } else {
            window.state_.current.focus = WindowFocus::Unfocused;
        }

        if (!window.focusStream_) {
            return;
        }

        WindowFocusChange event = {
            .target = window.handle_,
            .oldFocus = window.state_.previous.focus,
            .newFocus = window.state_.current.focus,
        };

        window.focusStream_->enqueue(event);
    }

    void Window::iconifyCallback(GLFWwindow* handle, int status) {
        Window& window = extract(handle);

        if (window.state_.current.status == WindowStatus::Inactive) {
            return;
        }

        window.state_.previous = window.state_.current;

        if (status) {
            window.state_.current.status = WindowStatus::Iconified;
        } else {
            window.state_.current.status = WindowStatus::Active;
        }

        if (!window.statusStream_) {
            return;
        }

        WindowStatusChange event = {
            .target = window.handle_,
            .oldStatus = window.state_.previous.status,
            .newStatus = window.state_.current.status,
        };

        window.statusStream_->enqueue(event);
    }

    void Window::closeCallback(GLFWwindow* handle) {
        Window& window = extract(handle);

        if (window.state_.current.status == WindowStatus::Inactive) {
            return;
        }

        window.state_.previous = window.state_.current;
        window.state_.current.status = WindowStatus::Inactive;

        if (!window.statusStream_) {
            return;
        }

        WindowStatusChange event = {
            .target = window.handle_,
            .oldStatus = window.state_.previous.status,
            .newStatus = window.state_.current.status,
        };

        window.statusStream_->enqueue(event);
    }
}
