#pragma once

#include "logical_device.hpp"

namespace lumina::renderer {
    class SwapchainManager final {
    public:
        SwapchainManager() = default;
        ~SwapchainManager();

        SwapchainManager(LogicalDevice& logicalDevice, SurfaceManager& surfaceManager);

        void destroy();

        [[nodiscard]] LogicalDevice& logicalDevice();
        [[nodiscard]] const LogicalDevice& logicalDevice() const;

    private:
        LogicalDevice* logicalDevice_ = nullptr;
        SurfaceManager* surfaceManager_ = nullptr;
    };
}
