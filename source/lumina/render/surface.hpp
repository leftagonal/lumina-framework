#pragma once

#include <lumina/render/instance.hpp>
#include <lumina/render/window.hpp>

namespace lumina::render {
    struct SurfaceInfo {
        Window& window;
        Instance& instance;
    };

    class Surface {
    public:
        Surface(const SurfaceInfo& info)
            : instance_(&info.instance), window_(&info.window) {
            VkResult result = glfwCreateWindowSurface(instance_->handle(), window_->handle(), nullptr, &surface_);

            if (result != VK_SUCCESS) {
                std::printf("error: failed to create window surface\n");
                std::exit(1);
            }
        }

        ~Surface() {
            if (surface_) {
                vkDestroySurfaceKHR(instance_->handle(), surface_, nullptr);
                surface_ = nullptr;
            }
        }

        [[nodiscard]] VkSurfaceKHR handle() const {
            return surface_;
        }

    private:
        VkSurfaceKHR surface_ = nullptr;
        Instance* instance_ = nullptr;
        Window* window_ = nullptr;
    };
}