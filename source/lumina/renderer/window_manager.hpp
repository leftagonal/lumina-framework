#pragma once

#include "window.hpp"

#include <span>
#include <vector>

namespace lumina::renderer {
    class WindowManager {
    public:
        WindowManager() = default;
        WindowManager(Instance& instance);
        ~WindowManager() = default;

        WindowManager(const WindowManager&) = delete;
        WindowManager(WindowManager&&) noexcept = default;

        WindowManager& operator=(const WindowManager&) = delete;
        WindowManager& operator=(WindowManager&&) noexcept = default;

        WindowHandle create(WindowInfo& info);
        void destroy(const WindowHandle& handle);

        [[nodiscard]] Instance& instance();
        [[nodiscard]] const Instance& instance() const;

        [[nodiscard]] Window& get(const WindowHandle& handle);
        [[nodiscard]] const Window& get(const WindowHandle& handle) const;
        [[nodiscard]] bool contains(const WindowHandle& handle) const;

        void clear();

        [[nodiscard]] std::span<Window> windows();
        [[nodiscard]] std::span<const Window> windows() const;

        [[nodiscard]] std::size_t count() const;
        [[nodiscard]] bool empty() const;

    private:
        Instance* instance_ = nullptr;
        std::vector<Window> windows_;
        std::vector<std::size_t> freeList_;
        std::size_t aliveCounter_ = 0;
    };
}
