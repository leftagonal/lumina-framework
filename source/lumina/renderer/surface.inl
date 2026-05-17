#pragma once

#include "instance.hpp"
#include "logical_device.hpp"
#include "surface.hpp"

namespace lumina::renderer {
    Surface::~Surface() {
        destroy();
    }

    Surface::Surface(Instance& instance, system::WindowManager& windowManager, const system::WindowHandle& windowHandle) {
        create(instance, windowManager, windowHandle);
    }

    Surface::Surface(Surface&& other) noexcept
        : instance_(other.instance_), window_(other.window_), surface_(other.surface_) {
        other.surface_ = nullptr;
        other.window_ = nullptr;
        other.instance_ = nullptr;
    }

    Surface& Surface::operator=(Surface&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        instance_ = other.instance_;
        window_ = other.window_;
        surface_ = other.surface_;

        other.surface_ = nullptr;
        other.window_ = nullptr;
        other.instance_ = nullptr;

        return *this;
    }

    Surface::operator bool() const {
        return valid();
    }

    void Surface::create(Instance& instance, system::WindowManager& windowManager, const system::WindowHandle& windowHandle) {
        if (valid()) {
            return;
        }

        instance_ = &instance;
        window_ = &windowManager.get(windowHandle);

        auto windowResource = system::accessors::WindowAccessor::window(*window_);
        auto instanceResource = accessors::InstanceAccessor::instance(*instance_);

        VkResult result = glfwCreateWindowSurface(instanceResource, windowResource, nullptr, &surface_);

        meta::assert(result == VK_SUCCESS, "failed to create Vulkan window surface: {}", core::Vulkan_errorString(result));
    }

    void Surface::destroy() {
        if (!valid()) {
            return;
        }

        auto instanceHandle = accessors::InstanceAccessor::instance(*instance_);

        vkDestroySurfaceKHR(instanceHandle, surface_, nullptr);

        surface_ = nullptr;
        instance_ = nullptr;
    }

    bool Surface::valid() const {
        return surface_ != nullptr;
    }

    Instance& Surface::instance() {
        return *instance_;
    }

    const Instance& Surface::instance() const {
        return *instance_;
    }

    system::Window& Surface::window() {
        return *window_;
    }

    const system::Window& Surface::window() const {
        return *window_;
    }

    std::vector<PresentMode> Surface::presentModes(const LogicalDevice& device) const {
        auto physicalDevice = accessors::LogicalDeviceAccessor::physicalDevice(device);

        std::uint32_t modeCount = 0;

        VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface_, &modeCount, nullptr);

        meta::assert(result == VK_SUCCESS, "failed to enumerate Vulkan surface present modes: {}", core::Vulkan_errorString(result));

        std::vector<VkPresentModeKHR> supported(modeCount);

        result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface_, &modeCount, supported.data());

        meta::assert(result == VK_SUCCESS, "failed to enumerate Vulkan surface present modes: {}", core::Vulkan_errorString(result));

        std::vector<PresentMode> presentModes;

        for (auto& candidate : supported) {
            if (converters::SurfaceEnumConverter::mappable(candidate)) {
                presentModes.emplace_back(converters::SurfaceEnumConverter::map(candidate));
            }
        }

        return presentModes;
    }

    std::vector<SurfaceFormat> Surface::formats(const LogicalDevice& device) const {
        auto physicalDevice = accessors::LogicalDeviceAccessor::physicalDevice(device);

        std::uint32_t formatCount = 0;

        VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface_, &formatCount, nullptr);

        meta::assert(result == VK_SUCCESS, "failed to enumerate Vulkan surface formats: {}", core::Vulkan_errorString(result));

        std::vector<VkSurfaceFormatKHR> supported(formatCount);

        result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface_, &formatCount, supported.data());

        meta::assert(result == VK_SUCCESS, "failed to enumerate Vulkan surface formats: {}", core::Vulkan_errorString(result));

        std::vector<SurfaceFormat> surfaceFormats;

        for (auto& candidate : supported) {
            bool formatMappable = converters::SurfaceEnumConverter::mappable(candidate.format);
            bool spaceMappable = converters::SurfaceEnumConverter::mappable(candidate.colorSpace);

            if (!formatMappable || !spaceMappable) {
                continue;
            }

            auto format = converters::SurfaceEnumConverter::map(candidate.format);
            auto space = converters::SurfaceEnumConverter::map(candidate.colorSpace);

            surfaceFormats.emplace_back(format, space);
        }

        return surfaceFormats;
    }

    SurfaceCapabilities Surface::capabilities(const LogicalDevice& device) const {
        auto physicalDevice = accessors::LogicalDeviceAccessor::physicalDevice(device);

        VkSurfaceCapabilitiesKHR reported = {};

        VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface_, &reported);

        meta::assert(result == VK_SUCCESS, "failed to get Vulkan surface capabilities: {}", core::Vulkan_errorString(result));

        SurfaceCapabilities capabilities = {
            .minimumImageCount = reported.minImageCount,
            .maximumImageCount = reported.maxImageCount,
            .minimumExtent = {reported.minImageExtent.width, reported.minImageExtent.height},
            .maximumExtent = {reported.maxImageExtent.width, reported.maxImageExtent.height},
            .currentExtent = {reported.currentExtent.width, reported.currentExtent.height},
        };

        bool badWidth = capabilities.currentExtent.width == 0xFFFFFFFF;
        bool badHeight = capabilities.currentExtent.height == 0xFFFFFFFF;

        if (badWidth || badHeight) {
            auto windowExtent = window_->extent();

            auto clampedWidth = std::clamp(windowExtent.width, capabilities.minimumExtent.width, capabilities.maximumExtent.width);
            auto clampedHeight = std::clamp(windowExtent.height, capabilities.minimumExtent.height, capabilities.maximumExtent.height);

            capabilities.currentExtent = {clampedWidth, clampedHeight};
        }

        return capabilities;
    }
}
