#pragma once

#include "surface_manager.hpp"

namespace lumina::renderer {
    /// @brief The mode of presentation a given swapchain is to use.
    enum class PresentMode {
        /// @brief Immediately displays the frame
        Immediate,

        /// @brief Waits for the next v-blank.
        Synchronous,

        /// @brief Overwrites the oldest, presents the newest on v-blank.
        Asynchronous,

        /// @brief Waits for the next v-blank, allows late submissions.
        Semisynchonous,
    };

    class LogicalDevice;

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

    private:
        VkSwapchainKHR swapchain_ = nullptr;
    };
}
