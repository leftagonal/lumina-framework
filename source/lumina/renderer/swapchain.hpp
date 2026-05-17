#pragma once

#include "queue.hpp"
#include "surface_manager.hpp"

namespace lumina::renderer {
    class LogicalDevice;
    class Queue;

    struct SwapchainQueueInfo {
        QueueFamilyDefinition graphicsFamily;
        QueueFamilyDefinition presentFamily;
    };

    struct SwapchainInfo {
        PresentMode presentMode;
        SurfaceFormat surfaceFormat;

        std::uint32_t minimumImageCount;
        std::uint32_t minimumFramesInFlight;
    };

    namespace accessors {
        class SwapchainAccessor;
    };

    class Swapchain final {
    public:
        Swapchain() = default;
        ~Swapchain();

        Swapchain(LogicalDevice& device, SurfaceManager& surfaceManager, const SwapchainInfo& info, const SwapchainQueueInfo& queueInfo, const system::WindowHandle& windowHandle);

        Swapchain(const Swapchain&) = delete;
        Swapchain(Swapchain&& other) noexcept;

        Swapchain& operator=(const Swapchain&) = delete;
        Swapchain& operator=(Swapchain&& other) noexcept;

        explicit operator bool() const;

        void create(LogicalDevice& device, SurfaceManager& surfaceManager, const SwapchainInfo& info, const SwapchainQueueInfo& queueInfo, const system::WindowHandle& windowHandle);
        void recreate(SurfaceManager& surfaceManager, const SwapchainInfo& info, const SwapchainQueueInfo& queueInfo, const system::WindowHandle& windowHandle);
        void destroy();

        [[nodiscard]] bool valid() const;

        [[nodiscard]] LogicalDevice& logicalDevice();
        [[nodiscard]] const LogicalDevice& logicalDevice() const;

    private:
        LogicalDevice* logicalDevice_ = nullptr;

        VkSwapchainKHR swapchain_ = nullptr;

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
