#pragma once

#include "surface_manager.hpp"

namespace lumina::renderer {

    class LogicalDevice;

    struct SwapchainInfo {
        SurfacePresentMode presentMode;
        SurfaceFormat format;
        SurfaceColourSpace colourSpace;
        std::size_t minimumImageCount;
        std::size_t minimumFramesInFlight;
    };

    namespace accessors {
        class SwapchainAccessor;
    };

    class Swapchain final {
    public:
        Swapchain() = default;
        ~Swapchain();

        Swapchain(LogicalDevice& device, SurfaceManager& surfaceManager, const system::WindowHandle& windowHandle);

        explicit operator bool() const;

        void create(LogicalDevice& device, SurfaceManager& surfaceManager, const system::WindowHandle& windowHandle);
        void destroy();

        [[nodiscard]] bool valid() const;

        [[nodiscard]] LogicalDevice& device();
        [[nodiscard]] const LogicalDevice& device() const;

        [[nodiscard]] system::WindowHandle windowHandle() const;

    private:
        LogicalDevice* logicalDevice_ = nullptr;

        VkSwapchainKHR swapchain_ = nullptr;
        system::WindowHandle windowHandle_;

        friend class accessors::SwapchainAccessor;
    };

    namespace accessors {
        class SwapchainAccessor {
        public:
            SwapchainAccessor() = delete;

            [[nodiscard]] static VkSwapchainKHR& swapchain(Swapchain& swapchain) {
                return swapchain.swapchain_;
            }

            [[nodiscard]] static const VkSwapchainKHR& swapchain(const Swapchain& swapchain) {
                return swapchain.swapchain_;
            }
        };
    }
}
