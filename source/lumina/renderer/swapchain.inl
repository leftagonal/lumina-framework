#pragma once

#include "logical_device.hpp"
#include "swapchain.hpp"

namespace lumina::renderer {
    Swapchain::~Swapchain() {
        destroy();
    }

    Swapchain::Swapchain(LogicalDevice& device, SurfaceManager& surfaceManager, const SwapchainInfo& info, const SwapchainQueueInfo& queueInfo, const system::WindowHandle& windowHandle) {
        create(device, surfaceManager, info, queueInfo, windowHandle);
    }

    Swapchain::Swapchain(Swapchain&& other) noexcept
        : logicalDevice_(other.logicalDevice_), swapchain_(other.swapchain_) {
        other.logicalDevice_ = nullptr;
        other.swapchain_ = nullptr;
    }

    Swapchain& Swapchain::operator=(Swapchain&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        logicalDevice_ = other.logicalDevice_;
        swapchain_ = other.swapchain_;

        other.logicalDevice_ = nullptr;
        other.swapchain_ = nullptr;

        return *this;
    }

    Swapchain::operator bool() const {
        return valid();
    }

    void Swapchain::create(LogicalDevice& device, SurfaceManager& surfaceManager, const SwapchainInfo& info, const SwapchainQueueInfo& queueInfo, const system::WindowHandle& windowHandle) {
        if (valid()) {
            return;
        }

        logicalDevice_ = &device;

        auto& surface = surfaceManager.get(windowHandle);
        auto surfaceResource = accessors::SurfaceAccessor::surface(surface);
        auto deviceResource = accessors::LogicalDeviceAccessor::device(*logicalDevice_);

        bool needsConcurrency = queueInfo.graphicsFamily.familyIndex != queueInfo.presentFamily.familyIndex;

        std::array<std::uint32_t, 2> familyIndices = {
            queueInfo.graphicsFamily.familyIndex,
            queueInfo.presentFamily.familyIndex,
        };

        auto extent = surface.capabilities(*logicalDevice_).currentExtent;

        VkSwapchainCreateInfoKHR createInfo = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .surface = surfaceResource,
            .minImageCount = info.minimumImageCount,
            .imageFormat = converters::SurfaceEnumConverter::map(info.surfaceFormat.imageFormat),
            .imageColorSpace = converters::SurfaceEnumConverter::map(info.surfaceFormat.colourSpace),
            .imageExtent = {extent.width, extent.height},
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = needsConcurrency ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = needsConcurrency ? static_cast<std::uint32_t>(familyIndices.size()) : 0,
            .pQueueFamilyIndices = needsConcurrency ? familyIndices.data() : nullptr,
            .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, // Change if mobile support matters
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = converters::SurfaceEnumConverter::map(info.presentMode),
            .clipped = true,
            .oldSwapchain = nullptr,
        };

        VkResult result = vkCreateSwapchainKHR(deviceResource, &createInfo, nullptr, &swapchain_);

        meta::assert(result == VK_SUCCESS, "failed to create swapchain: {}", core::Vulkan_errorString(result));
    }

    void Swapchain::recreate(SurfaceManager& surfaceManager, const SwapchainInfo& info, const SwapchainQueueInfo& queueInfo, const system::WindowHandle& windowHandle) {
        auto& surface = surfaceManager.get(windowHandle);
        auto surfaceResource = accessors::SurfaceAccessor::surface(surface);
        auto deviceResource = accessors::LogicalDeviceAccessor::device(*logicalDevice_);

        bool needsConcurrency = queueInfo.graphicsFamily.familyIndex != queueInfo.presentFamily.familyIndex;

        std::array<std::uint32_t, 2> familyIndices = {
            queueInfo.graphicsFamily.familyIndex,
            queueInfo.presentFamily.familyIndex,
        };

        auto extent = surface.capabilities(*logicalDevice_).currentExtent;

        VkSwapchainCreateInfoKHR createInfo = {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .surface = surfaceResource,
            .minImageCount = info.minimumImageCount,
            .imageFormat = converters::SurfaceEnumConverter::map(info.surfaceFormat.imageFormat),
            .imageColorSpace = converters::SurfaceEnumConverter::map(info.surfaceFormat.colourSpace),
            .imageExtent = {extent.width, extent.height},
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = needsConcurrency ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = needsConcurrency ? static_cast<std::uint32_t>(familyIndices.size()) : 0,
            .pQueueFamilyIndices = needsConcurrency ? familyIndices.data() : nullptr,
            .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR, // Change if mobile support matters
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = converters::SurfaceEnumConverter::map(info.presentMode),
            .clipped = true,
            .oldSwapchain = swapchain_,
        };

        VkSwapchainKHR newSwapchain = nullptr;
        VkResult result = vkCreateSwapchainKHR(deviceResource, &createInfo, nullptr, &newSwapchain);

        vkDestroySwapchainKHR(deviceResource, swapchain_, nullptr);

        swapchain_ = newSwapchain;

        meta::assert(result == VK_SUCCESS, "failed to create swapchain: {}", core::Vulkan_errorString(result));
    }

    void Swapchain::destroy() {
        if (!valid()) {
            return;
        }

        // TODO: wait on fences

        auto deviceResource = accessors::LogicalDeviceAccessor::device(*logicalDevice_);

        vkDestroySwapchainKHR(deviceResource, swapchain_, nullptr);

        logicalDevice_ = nullptr;
        swapchain_ = nullptr;
    }

    bool Swapchain::valid() const {
        return swapchain_ != nullptr;
    }

    LogicalDevice& Swapchain::logicalDevice() {
        return *logicalDevice_;
    }

    const LogicalDevice& Swapchain::logicalDevice() const {
        return *logicalDevice_;
    }
}
