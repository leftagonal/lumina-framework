#pragma once

#include <lumina/core/glfw.hpp>
#include <lumina/events/socket.hpp>
#include <lumina/meta/diff.hpp>
#include <lumina/meta/extent.hpp>
#include <lumina/meta/handle.hpp>

namespace lumina::system {
    class Window;
    using WindowHandle = meta::Handle<Window>;
    using WindowTitle = std::string_view;
    using WindowExtent = meta::Extent2D<std::uint32_t>;

    struct WindowFeatures {
        bool resizable;
    };

    struct WindowInfo {
        WindowTitle title;
        WindowExtent extent;
        WindowFeatures features;
    };

    enum class WindowVisibility {
        Onscreen,
        Iconified,
    };

    enum class WindowDisplayMode {
        Windowed,
        Fullscreen,
        Exclusive,
    };

    enum class WindowFocus {
        Focused,
        Unfocused,
    };

    enum class WindowValidity {
        Valid,
        Invalid,
    };

    struct WindowExtentChangeEvent {
        WindowHandle target;
        meta::Diff<WindowExtent> diff;
    };

    struct WindowVisibilityChangeEvent {
        WindowHandle target;
        meta::Diff<WindowVisibility> diff;
    };

    struct WindowDisplayModeChangeEvent {
        WindowHandle target;
        meta::Diff<WindowDisplayMode> diff;
    };

    struct WindowFocusChangeEvent {
        WindowHandle target;
        meta::Diff<WindowFocus> diff;
    };

    struct WindowValidityChangeEvent {
        WindowHandle target;
        meta::Diff<WindowValidity> diff;
    };

    using WindowExtentChangeSocket = events::Socket<WindowExtentChangeEvent>;
    using WindowVisibilityChangeSocket = events::Socket<WindowVisibilityChangeEvent>;
    using WindowDisplayModeChangeSocket = events::Socket<WindowDisplayModeChangeEvent>;
    using WindowFocusChangeSocket = events::Socket<WindowFocusChangeEvent>;
    using WindowValidityChangeSocket = events::Socket<WindowValidityChangeEvent>;

    struct WindowEventSockets {
        WindowExtentChangeSocket extentSocket;
        WindowVisibilityChangeSocket visibilitySocket;
        WindowDisplayModeChangeSocket displayModeSocket;
        WindowFocusChangeSocket focusSocket;
        WindowValidityChangeSocket validitySocket;
    };

    struct WindowState {
        std::string title = "";
        WindowExtent extent = {0, 0};
        WindowVisibility visibility;
        WindowDisplayMode displayMode;
        WindowFocus focus;
        WindowValidity validity = WindowValidity::Invalid;
    };

    namespace accessors {
        class WindowAccessor;
    }

    class Window final {
    public:
        Window() = default;
        ~Window();

        Window(const WindowHandle& handle, const WindowInfo& info, const WindowEventSockets& sockets);

        Window(const Window&) = delete;
        Window(Window&& other) noexcept;

        Window& operator=(const Window&) = delete;
        Window& operator=(Window&& other) noexcept;

        explicit operator bool() const;

        void create(const WindowHandle& handle, const WindowInfo& info, const WindowEventSockets& sockets);
        void destroy();

        [[nodiscard]] bool valid() const;
        [[nodiscard]] WindowHandle handle() const;
        [[nodiscard]] WindowExtent extent() const;

    private:
        GLFWwindow* window_ = nullptr;
        meta::Diff<WindowState> state_ = {};
        WindowEventSockets sockets_ = {};
        WindowHandle handle_;

        [[nodiscard]] static Window& extract(GLFWwindow* handle);

        static void extentChangeCallback(GLFWwindow* handle, int width, int height);
        static void focusChangeCallback(GLFWwindow* handle, int status);
        static void visibilityChangeCallback(GLFWwindow* handle, int status);
        static void validityChangeCallback(GLFWwindow* handle);

        friend class accessors::WindowAccessor;
    };

    namespace accessors {
        class WindowAccessor final {
        public:
            WindowAccessor() = delete;

            [[nodiscard]] static GLFWwindow*& window(Window& window) {
                return window.window_;
            }

            [[nodiscard]] static GLFWwindow* const& window(const Window& window) {
                return window.window_;
            }
        };
    }
}
