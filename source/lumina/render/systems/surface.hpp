#pragma once

#include "../meta/resource.hpp"
#include "instance.hpp"
#include "window.hpp"

namespace lumina::render {
    /**
     * @brief Represents a renderable region of a window.
     *
     * Note that this may not be available on every system. You must successfully
     * create your instance with the 'surfaces' feature enabled.
     */
    class Surface : public Resource<VkSurfaceKHR> {
    public:
        Surface() = default;

        /**
         * @brief Construct a new surface.
         *
         * @param window The window to create a surface for.
         * @param instance The connection to the driver.
         */
        Surface(Window& window, Instance& instance)
            : instance_(&instance), window_(&window) {
            VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &resource());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to create window surface\n");
                std::exit(1);
            }
        }

        /**
         * @brief Destroy the surface.
         *
         */
        ~Surface() {
            destroy();
        }

        Surface(Surface&&) noexcept = default;

        Surface& operator=(Surface&&) noexcept = default;

        /**
         * @brief Construct a new surface.
         *
         * @param window The window to create a surface for.
         * @param instance The connection to the driver.
         */
        void create(Window& window, Instance& instance) {
            if (*this) {
                return;
            }

            instance_ = &instance;
            window_ = &window;

            VkResult result = glfwCreateWindowSurface(instance, window, nullptr, &resource());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to create window surface\n");
                std::exit(1);
            }
        }

        /**
         * @brief Destroy the surface.
         *
         */
        void destroy() {
            if (!*this) {
                return;
            }

            vkDestroySurfaceKHR(*instance_, resource(), nullptr);

            window_ = nullptr;
            instance_ = nullptr;

            invalidate();
        }

        /**
         * @brief Provides the surface's current extent.
         * 
         * @return Extent2D The extent of the surface.
         */
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

            VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, family.definition.index, resource(), &supported);

            if (result != VK_SUCCESS) {
                std::printf("error: failed to query queue family present support\n");
                std::exit(1);
            }

            return supported;
        }

    private:
        Instance* instance_ = nullptr;
        Window* window_ = nullptr;
    };
}