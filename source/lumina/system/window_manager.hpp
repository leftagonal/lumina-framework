#pragma once

#include "window.hpp"

#include <vector>

namespace lumina::system {
    class Instance;

    struct WindowManagerWindowCreateEvent {
        WindowHandle handle;
    };

    struct WindowManagerWindowDestroyEvent {
        WindowHandle handle;
    };

    using WindowManagerWindowCreateSocket = events::Socket<WindowManagerWindowCreateEvent>;
    using WindowManagerWindowDestroySocket = events::Socket<WindowManagerWindowDestroyEvent>;

    struct WindowManagerEventSockets {
        WindowManagerWindowCreateSocket createSocket;
        WindowManagerWindowDestroySocket destroySocket;
    };

    class WindowManager {
    public:
        WindowManager() = default;
        ~WindowManager();

        WindowManager(Instance& instance, const WindowManagerEventSockets& sockets);

        WindowManager(const WindowManager&) = delete;
        WindowManager(WindowManager&& other) noexcept;

        WindowManager& operator=(const WindowManager&) = delete;
        WindowManager& operator=(WindowManager&& other) noexcept;

        explicit operator bool() const;

        void create(Instance& instance, const WindowManagerEventSockets& sockets);
        void destroy();

        [[nodiscard]] bool valid() const;

        [[nodiscard]] WindowHandle insert(const WindowInfo& info, const WindowEventSockets& sockets);
        void remove(const WindowHandle& handle);

        [[nodiscard]] Window& get(const WindowHandle& handle);
        [[nodiscard]] const Window& get(const WindowHandle& handle) const;

        [[nodiscard]] bool contains(const WindowHandle& handle) const;

        [[nodiscard]] Instance& instance();
        [[nodiscard]] const Instance& instance() const;

        [[nodiscard]] std::vector<Window>& windows();
        [[nodiscard]] const std::vector<Window>& windows() const;

        [[nodiscard]] std::size_t count() const;
        [[nodiscard]] bool empty() const;

        [[nodiscard]] WindowManagerEventSockets& sockets();
        [[nodiscard]] const WindowManagerEventSockets& sockets() const;

    private:
        Instance* instance_ = nullptr;

        std::vector<Window> windows_;
        std::vector<std::size_t> freeList_;
        WindowManagerEventSockets sockets_;
    };
}
