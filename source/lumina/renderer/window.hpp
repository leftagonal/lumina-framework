#pragma once

#include <lumina/events/socket.hpp>

#include "glfw.hpp"
#include "handle.hpp"

namespace lumina::renderer {
    struct Extent2D {
        std::size_t width;
        std::size_t height;
    };

    enum class WindowStatus {
        Inactive,
        Iconified,
        Active,
    };

    enum class WindowFocus {
        Focused,
        Unfocused,
    };

    struct WindowFeatures {
        bool resizable;
    };

    struct WindowTag;
    using WindowHandle = Handle<WindowTag>;

    struct WindowResize {
        WindowHandle target;

        Extent2D oldExtent;
        Extent2D newExtent;
    };

    struct WindowStatusChange {
        WindowHandle target;

        WindowStatus oldStatus;
        WindowStatus newStatus;
    };

    struct WindowFocusChange {
        WindowHandle target;

        WindowFocus oldFocus;
        WindowFocus newFocus;
    };

    using WindowSizeSocket = events::Socket<WindowResize>;
    using WindowStatusSocket = events::Socket<WindowStatusChange>;
    using WindowFocusSocket = events::Socket<WindowFocusChange>;

    struct WindowInfo {
        std::string_view title;
        Extent2D extent;
        WindowFeatures features;
        WindowSizeSocket sizeSocket;
        WindowStatusSocket statusSocket;
        WindowFocusSocket focusSocket;
    };

    class Instance;

    namespace accessors {
        class WindowAccessor;
    }

    class Window {
    public:
        struct WindowState {
            std::string title;
            Extent2D extent;
            WindowFocus focus;
            WindowStatus status;
        };

        Window() = default;
        Window(const WindowHandle& handle, Instance& instance, WindowInfo& info);
        ~Window();

        Window(const Window&) = delete;
        Window(Window&&) noexcept;

        Window& operator=(const Window&) = delete;
        Window& operator=(Window&&) noexcept;

        [[nodiscard]] WindowState current() const;
        [[nodiscard]] WindowState previous() const;

        void resize(const Extent2D& extent);
        void rename(std::string_view title);
        void resurface();
        void destroy();

        [[nodiscard]] bool valid() const;
        explicit operator bool() const;

    private:
        template <typename T>
        struct Diff {
            T current;
            T previous;
        };

        Instance* instance_ = nullptr;
        WindowSizeSocket sizeSocket_;
        WindowStatusSocket statusSocket_;
        WindowFocusSocket focusSocket_;

        GLFWwindow* window_ = nullptr;
        VkSurfaceKHR surface_ = nullptr;
        WindowHandle handle_ = {};

        Diff<WindowState> state_;

        [[nodiscard]] static Window& extract(GLFWwindow* handle);

        static void framebufferResizeCallback(GLFWwindow* handle, int width, int height);
        static void focusCallback(GLFWwindow* handle, int state);
        static void iconifyCallback(GLFWwindow* handle, int state);
        static void closeCallback(GLFWwindow* handle);

        friend class accessors::WindowAccessor;
    };

    namespace accessors {
        class WindowAccessor {
        public:
            WindowAccessor() = delete;

            [[nodiscard]] static GLFWwindow*& window(Window& window) {
                return window.window_;
            }

            [[nodiscard]] static GLFWwindow* const& window(const Window& window) {
                return window.window_;
            }

            [[nodiscard]] static VkSurfaceKHR& surface(Window& window) {
                return window.surface_;
            }

            [[nodiscard]] static const VkSurfaceKHR& surface(const Window& window) {
                return window.surface_;
            }
        };
    }
}
