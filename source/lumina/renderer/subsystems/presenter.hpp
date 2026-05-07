
#pragma once

#include "../glfw.hpp"
#include "../handle.hpp"

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
        WindowStatus status;
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
        struct PresenterAccessor;
    }

    namespace subsystems {
        class Device;

        class Presenter {
            struct PresenterBinding {
                std::size_t windowIndex;
                Presenter* presenter;
            };

        public:
            Presenter(Instance& instance);
            ~Presenter();

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

            [[nodiscard]] Device& device() {
                return *device_;
            }

            [[nodiscard]] const Device& device() const {
                return *device_;
            }

            [[nodiscard]] bool bound() const {
                return device_ != nullptr;
            }

            [[nodiscard]] std::size_t count() const {
                return windows_.size();
            }

            [[nodiscard]] bool debugging() const {
                return debugging_;
            }

            void reset();

            [[nodiscard]] WindowAttributes attributes(const WindowHandle& handle) const {
                auto& window = windows_[handle.id()];

                return WindowAttributes{
                    .status = window.status,
                    .focus = window.focus,
                    .title = window.title,
                    .extent = window.extent,
                    .resizable = window.resizable,
                };
            }

            // TODO: setters for window attributes

        private:
            struct WindowUserPointerData {
                WindowHandle handle;
                Presenter* presenter;
            };

            struct WindowCache {
                WindowStatus status;
                WindowStatus lastStatus;
                WindowFocus focus;
                std::string title;
                Extent2D extent;
                bool resizable;
                GLFWwindow* resource;

                std::unique_ptr<WindowUserPointerData> userPointer;
            };

            std::vector<WindowCache> windows_;
            std::vector<VkSurfaceKHR> surfaces_;
            std::vector<VkSwapchainKHR> swapchains_;
            std::vector<std::size_t> freeList_;

            bool debugging_;

            Instance* instance_ = nullptr;
            Device* device_ = nullptr;

            friend struct accessors::PresenterAccessor;

            static void resizeCallback(GLFWwindow* window, int width, int height);
            static void closeCallback(GLFWwindow* window);
            static void iconifyCallback(GLFWwindow* window, int status);
            static void focusCallback(GLFWwindow* window, int status);
        };
    }

    namespace accessors {
        struct PresenterAccessor {
            [[nodiscard]] static GLFWwindow*& window(subsystems::Presenter& presenter, const WindowHandle& handle) {
                return presenter.windows_[handle.id()].resource;
            }

            [[nodiscard]] static VkSurfaceKHR& surface(subsystems::Presenter& presenter, const WindowHandle& handle) {
                return presenter.surfaces_[handle.id()];
            }

            [[nodiscard]] static VkSwapchainKHR& swapchain(subsystems::Presenter& presenter, const WindowHandle& handle) {
                return presenter.swapchains_[handle.id()];
            }

            [[nodiscard]] static const GLFWwindow* const& window(const subsystems::Presenter& presenter, const WindowHandle& handle) {
                return presenter.windows_[handle.id()].resource;
            }

            [[nodiscard]] static const VkSurfaceKHR& surface(const subsystems::Presenter& presenter, const WindowHandle& handle) {
                return presenter.surfaces_[handle.id()];
            }

            [[nodiscard]] static const VkSwapchainKHR& swapchain(const subsystems::Presenter& presenter, const WindowHandle& handle) {
                return presenter.swapchains_[handle.id()];
            }
        };
    }
}