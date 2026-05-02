#pragma once

#include <lumina/render/instance.hpp>
#include <lumina/render/window.hpp>

namespace lumina::render {
    class Surface {
    public:
        Surface(Window& window, Instance& instance)
            : instance_(&instance), window_(&window) {
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
                window_ = nullptr;
                instance_ = nullptr;
            }
        }

        [[nodiscard]] VkSurfaceKHR handle() const {
            return surface_;
        }

        [[nodiscard]] Extent2D extent() const {
            return window_->extent();
        }

        [[nodiscard]] std::vector<QueueFamily> supportedFamilies(PhysicalDevice& physicalDevice) {
            auto families = physicalDevice.queueFamilies();
            std::vector<QueueFamily> supportedFamilies;

            for (auto& family : families) {
                VkBool32 supported = supportsFamily(physicalDevice, family);

                if (supported) {
                    supportedFamilies.emplace_back(family);
                }
            }

            return supportedFamilies;
        }

        [[nodiscard]] bool supportsFamily(PhysicalDevice& physicalDevice, const QueueFamily& family) {
            VkBool32 supported = VK_FALSE;

            VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice.handle(), family.definition.index, surface_, &supported);

            if (result != VK_SUCCESS) {
                std::printf("error: failed to query queue family present support\n");
                std::exit(1);
            }

            return supported;
        }

    private:
        VkSurfaceKHR surface_ = nullptr;

        Instance* instance_;
        Window* window_;
    };
}