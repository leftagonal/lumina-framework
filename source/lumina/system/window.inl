#pragma once

#include <lumina/meta/exceptions.hpp>

#include "instance.hpp"
#include "window.hpp"

namespace lumina::system {
    inline Window::~Window() {
        destroy();
    }

    inline Window::Window(const WindowHandle& handle, const WindowInfo& info, const WindowEventSockets& sockets) {
        create(handle, info, sockets);
    }

    inline Window::Window(Window&& other) noexcept
        : window_(other.window_), state_(other.state_), sockets_(other.sockets_), handle_(other.handle_) {
        if (window_) {
            glfwSetWindowUserPointer(window_, this);
        }

        other.sockets_.extentSocket.disconnect();
        other.sockets_.visibilitySocket.disconnect();
        other.sockets_.focusSocket.disconnect();
        other.sockets_.displayModeSocket.disconnect();
        other.sockets_.validitySocket.disconnect();
    }

    inline Window& Window::operator=(Window&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        window_ = other.window_;
        state_ = other.state_;
        sockets_ = other.sockets_;
        handle_ = other.handle_;

        if (window_) {
            glfwSetWindowUserPointer(window_, this);
        }

        other.sockets_.extentSocket.disconnect();
        other.sockets_.visibilitySocket.disconnect();
        other.sockets_.focusSocket.disconnect();
        other.sockets_.displayModeSocket.disconnect();
        other.sockets_.validitySocket.disconnect();

        return *this;
    }

    inline Window::operator bool() const {
        return valid();
    }

    inline void Window::create(const WindowHandle& handle, const WindowInfo& info, const WindowEventSockets& sockets) {
        if (valid()) {
            return;
        }

        state_.current.title = info.title;
        state_.current.extent = info.extent;

        sockets_ = sockets;
        handle_ = handle;

        int width = static_cast<int>(info.extent.width);
        int height = static_cast<int>(info.extent.height);

        window_ = glfwCreateWindow(width, height, info.title.data(), nullptr, nullptr);

        meta::assert(window_ != nullptr, "failed to create window");

        glfwSetWindowUserPointer(window_, this);

        glfwSetFramebufferSizeCallback(window_, extentChangeCallback);
        glfwSetWindowFocusCallback(window_, focusChangeCallback);
        glfwSetWindowIconifyCallback(window_, visibilityChangeCallback);
        glfwSetWindowCloseCallback(window_, validityChangeCallback);
    }

    inline void Window::destroy() {
        if (!valid()) {
            return;
        }

        glfwDestroyWindow(window_);

        window_ = nullptr;

        sockets_.extentSocket.disconnect();
        sockets_.visibilitySocket.disconnect();
        sockets_.focusSocket.disconnect();
        sockets_.displayModeSocket.disconnect();
        sockets_.validitySocket.disconnect();
    }

    inline bool Window::valid() const {
        return window_ != nullptr;
    }

    inline WindowHandle Window::handle() const {
        return handle_; 
    }

    Window& Window::extract(GLFWwindow* handle) {
        void* userPointer = glfwGetWindowUserPointer(handle);

        meta::cassert(userPointer != nullptr, "window lost");

        return *static_cast<Window*>(userPointer);
    }

    void Window::extentChangeCallback(GLFWwindow* handle, int width, int height) {
        Window& window = extract(handle);

        window.state_.previous.extent = window.state_.current.extent;

        window.state_.current.extent = {
            static_cast<std::uint32_t>(width),
            static_cast<std::uint32_t>(height),
        };

        if (!window.sockets_.extentSocket) {
            return;
        }

        WindowExtentChangeEvent event = {
            .target = window.handle_,
            .diff = {
                window.state_.previous.extent,
                window.state_.current.extent,
            },
        };

        window.sockets_.extentSocket->enqueue(event);
    }

    void Window::focusChangeCallback(GLFWwindow* handle, int status) {
        Window& window = extract(handle);

        window.state_.previous.focus = window.state_.current.focus;

        if (status) {
            window.state_.current.focus = WindowFocus::Focused;
        } else {
            window.state_.current.focus = WindowFocus::Unfocused;
        }

        if (!window.sockets_.focusSocket) {
            return;
        }

        WindowFocusChangeEvent event = {
            .target = window.handle_,
            .diff = {
                window.state_.previous.focus,
                window.state_.current.focus,
            },
        };

        window.sockets_.focusSocket->enqueue(event);
    }

    void Window::visibilityChangeCallback(GLFWwindow* handle, int status) {
        Window& window = extract(handle);

        window.state_.previous.visibility = window.state_.current.visibility;

        if (status) {
            window.state_.current.visibility = WindowVisibility::Iconified;
        } else {
            window.state_.current.visibility = WindowVisibility::Onscreen;
        }

        if (!window.sockets_.visibilitySocket) {
            return;
        }

        WindowVisibilityChangeEvent event = {
            .target = window.handle_,
            .diff = {
                window.state_.previous.visibility,
                window.state_.current.visibility,
            },
        };

        window.sockets_.visibilitySocket->enqueue(event);
    }

    void Window::validityChangeCallback(GLFWwindow* handle) {
        Window& window = extract(handle);

        window.state_.previous.validity = window.state_.current.validity;
        window.state_.current.validity = WindowValidity::Invalid;

        if (!window.sockets_.validitySocket) {
            return;
        }

        WindowValidityChangeEvent event = {
            .target = window.handle_,
            .diff = {
                window.state_.previous.validity,
                window.state_.current.validity,
            },
        };

        window.sockets_.validitySocket->enqueue(event);
    }
}
