
#pragma once

#include "glfw.hpp"
#include "handle.hpp"

#include <vector>

namespace lumina::renderer {
    class Instance;

    namespace subsystems {
        struct WindowTag;
    }

    using WindowHandle = Handle<subsystems::WindowTag>;

    enum class WindowStatus {
        // inactive windows are ready for destruction. they are valid until destroyed.
        Inactive,

        // iconified windows are not displayed but still exist, and can be restored.
        Iconified,

        // active windows are on-screen.
        Active,
    };

    enum class WindowFocus {
        // the window is currently being interacted with
        Focused,

        // the window is currently in the background and not being interacted with
        Unfocused,
    };

    struct Extent2D {
        std::uint32_t width;
        std::uint32_t height;
    };

    struct WindowInfo {
        std::string_view title;
        Extent2D extent;
        bool resizable;
    };

    struct WindowAttributes {
        WindowStatus status;
        WindowFocus focus;
        std::string_view title;
        Extent2D extent;
        bool resizable;
    };

    namespace accessors {
        struct DisplayServerAccessor;
    }

    class LogicalDevice;

    /**
     * @brief Owns and operates windows, providing access to their renderable regions.
     *
     */
    class DisplayServer {
    public:
        DisplayServer(Instance& instance);
        ~DisplayServer();

        [[nodiscard]] WindowHandle create(const WindowInfo& info);

        void destroy(const WindowHandle& handle);

        [[nodiscard]] bool exists(const WindowHandle& handle) const {
            return handle.valid() && handle.id() < windows_.size();
        }

        [[nodiscard]] Instance& instance() {
            return *instance_;
        }

        [[nodiscard]] const Instance& instance() const {
            return *instance_;
        }

        [[nodiscard]] LogicalDevice& device() {
            return *device_;
        }

        [[nodiscard]] const LogicalDevice& device() const {
            return *device_;
        }

        [[nodiscard]] bool bound() const {
            return device_ != nullptr;
        }

        [[nodiscard]] std::size_t count() const {
            return windows_.size();
        }

        [[nodiscard]] bool debugging() const {
            return validation_;
        }

        void reset();

        [[nodiscard]] WindowAttributes attributes(const WindowHandle& handle) const {
            auto& cache = caches_[handle.id()];

            return WindowAttributes{
                .status = cache.status,
                .focus = cache.focus,
                .title = cache.title,
                .extent = cache.extent,
                .resizable = cache.resizable,
            };
        }

        // TODO: setters for window attributes

    private:
        struct WindowBinding {
            WindowHandle handle;
            DisplayServer& server;
        };

        struct WindowCache {
            WindowStatus status;
            WindowStatus lastStatus;
            WindowFocus focus;
            std::string title;
            Extent2D extent;
            bool resizable;
        };

        std::vector<WindowCache> caches_;
        std::vector<GLFWwindow*> windows_;
        std::vector<VkSurfaceKHR> surfaces_;
        std::vector<VkSwapchainKHR> swapchains_;
        std::vector<std::size_t> freeList_;

        bool validation_;

        Instance* instance_ = nullptr;
        LogicalDevice* device_ = nullptr;

        friend struct accessors::DisplayServerAccessor;

        static void resizeCallback(GLFWwindow* window, int width, int height);
        static void closeCallback(GLFWwindow* window);
        static void iconifyCallback(GLFWwindow* window, int status);
        static void focusCallback(GLFWwindow* window, int status);
    };

    namespace accessors {
        struct DisplayServerAccessor {
            [[nodiscard]] static GLFWwindow*& window(DisplayServer& presenter, const WindowHandle& handle) {
                return presenter.windows_[handle.id()];
            }

            [[nodiscard]] static VkSurfaceKHR& surface(DisplayServer& presenter, const WindowHandle& handle) {
                return presenter.surfaces_[handle.id()];
            }

            [[nodiscard]] static VkSwapchainKHR& swapchain(DisplayServer& presenter, const WindowHandle& handle) {
                return presenter.swapchains_[handle.id()];
            }

            [[nodiscard]] static const GLFWwindow* const& window(const DisplayServer& presenter, const WindowHandle& handle) {
                return presenter.windows_[handle.id()];
            }

            [[nodiscard]] static const VkSurfaceKHR& surface(const DisplayServer& presenter, const WindowHandle& handle) {
                return presenter.surfaces_[handle.id()];
            }

            [[nodiscard]] static const VkSwapchainKHR& swapchain(const DisplayServer& presenter, const WindowHandle& handle) {
                return presenter.swapchains_[handle.id()];
            }
        };
    }
}
