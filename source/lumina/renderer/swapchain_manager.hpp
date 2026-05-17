#pragma once

#include "swapchain.hpp"

namespace lumina::renderer {
    class LogicalDevice;

    class SwapchainManager final {
    public:
        SwapchainManager() = default;
        ~SwapchainManager();

        SwapchainManager(LogicalDevice& logicalDevice, SurfaceManager& surfaceManager);

        SwapchainManager(const SwapchainManager&) = delete;
        SwapchainManager(SwapchainManager&&) noexcept;

        SwapchainManager& operator=(const SwapchainManager&) = delete;
        SwapchainManager& operator=(SwapchainManager&&) noexcept;

        explicit operator bool() const;

        void create(LogicalDevice& logicalDevice, SurfaceManager& surfaceManager);
        void destroy();

        void insert(const system::WindowHandle& target, const SwapchainInfo& info, const SwapchainQueueInfo& queueInfo);

        [[nodiscard]] bool valid() const;
        [[nodiscard]] LogicalDevice& logicalDevice();
        [[nodiscard]] const LogicalDevice& logicalDevice() const;

    private:
        LogicalDevice* logicalDevice_ = nullptr;
        SurfaceManager* surfaceManager_ = nullptr;

        std::vector<Swapchain> swapchains_;

        events::Token destroyToken_;

        void onSurfaceDestroy(const SurfaceManagerSurfaceDestroyEvent& event);
    };
}
