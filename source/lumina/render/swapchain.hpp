#pragma once

#include "device.hpp"
#include "image.hpp"
#include "surface.hpp"

namespace lumina::render {
    enum class SwapchainPresentMode {
        Synchronous,
        RelaxedSynchronous,
        Asynchronous,
        Immediate,
    };

    struct SwapchainConfig {
        SwapchainPresentMode presentMode;
        std::uint32_t minimumImageCount;
    };

    class Swapchain {
        struct SurfaceCapabilities {
            std::uint32_t minimumImageCount;
            VkExtent2D extent;
            VkSurfaceTransformFlagBitsKHR transform;
            VkCompositeAlphaFlagBitsKHR compositeAlpha;
        };

    public:
        Swapchain(Device& device, Surface& surface, Queue& graphicsQueue, Queue& presentQueue, const SwapchainConfig& config)
            : device_(&device), surface_(&surface) {
            SurfaceCapabilities capabilities = getSurfaceCapabilities(config);
            VkSurfaceFormatKHR surfaceFormat = selectSurfaceFormat();

            VkSwapchainCreateInfoKHR createInfo = {
                .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .pNext = nullptr,
                .flags = 0,
                .surface = surface_->handle(),
                .minImageCount = capabilities.minimumImageCount,
                .imageFormat = surfaceFormat.format,
                .imageColorSpace = surfaceFormat.colorSpace,
                .imageExtent = capabilities.extent,
                .imageArrayLayers = 1,
                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                .imageSharingMode = VK_SHARING_MODE_CONCURRENT,
                .pQueueFamilyIndices = nullptr,
                .preTransform = capabilities.transform,
                .compositeAlpha = capabilities.compositeAlpha,
                .presentMode = selectPresentMode(config),
                .clipped = VK_TRUE,
                .oldSwapchain = nullptr,
            };

            QueueAssignment graphicsAssignment = graphicsQueue.assignment();
            QueueAssignment presentAssignment = presentQueue.assignment();

            bool needsExclusive = graphicsAssignment.familyIndex != presentAssignment.familyIndex;

            std::array<std::uint32_t, 2> familyIndices = {
                graphicsAssignment.familyIndex,
                presentAssignment.familyIndex,
            };

            if (needsExclusive) {
                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                createInfo.pQueueFamilyIndices = familyIndices.data();
            }

            VkResult result = vkCreateSwapchainKHR(device_->handle(), &createInfo, nullptr, &swapchain_);

            if (result != VK_SUCCESS) {
                std::printf("error: failed to create swapchain\n");
                std::exit(1);
            }

            std::uint32_t swapchainImageCount = 0;

            result = vkGetSwapchainImagesKHR(device_->handle(), swapchain_, &swapchainImageCount, nullptr);

            if (result != VK_SUCCESS) {
                std::printf("error: failed to retrieve swapchain images\n");
                std::exit(1);
            }

            std::vector<VkImage> images(swapchainImageCount);
            images_.reserve(swapchainImageCount);

            result = vkGetSwapchainImagesKHR(device_->handle(), swapchain_, &swapchainImageCount, images.data());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to retrieve swapchain images\n");
                std::exit(1);
            }

            for (auto& image : images) {
                images_.emplace_back(image);
            }
        }

        ~Swapchain() {
            if (swapchain_) {
                vkDestroySwapchainKHR(device_->handle(), swapchain_, nullptr);
                swapchain_ = nullptr;
            }
        }

        [[nodiscard]] std::span<Image> images() {
            return images_;
        }

        [[nodiscard]] std::span<const Image> images() const {
            return images_;
        }

    private:
        VkSwapchainKHR swapchain_ = nullptr;

        Device* device_;
        Surface* surface_;

        std::vector<Image> images_;

        [[nodiscard]] static VkCompositeAlphaFlagBitsKHR selectCompositeAlpha(VkCompositeAlphaFlagsKHR supported) {
            static constexpr VkCompositeAlphaFlagBitsKHR candidates[] = {
                VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
                VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
                VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            };

            for (auto candidate : candidates) {
                if (supported & candidate) {
                    return candidate;
                }
            }

            return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        }

        [[nodiscard]] SurfaceCapabilities getSurfaceCapabilities(const SwapchainConfig& config) {
            VkPhysicalDevice physicalDevice = device_->physicalDevice().handle();
            VkSurfaceKHR surface = surface_->handle();

            VkSurfaceCapabilitiesKHR surfaceCapabilities;

            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

            return {
                .minimumImageCount = std::clamp(config.minimumImageCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount),
                .extent = surfaceCapabilities.currentExtent,
                .transform = surfaceCapabilities.currentTransform,
                .compositeAlpha = selectCompositeAlpha(surfaceCapabilities.supportedCompositeAlpha),
            };
        }

        [[nodiscard]] VkPresentModeKHR selectPresentMode(const SwapchainConfig& config) {
            VkPhysicalDevice physicalDevice = device_->physicalDevice().handle();
            VkSurfaceKHR surface = surface_->handle();

            std::uint32_t presentModeCount = 0;

            VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

            if (result != VK_SUCCESS) {
                std::printf("error: failed to query supported present modes\n");
                std::exit(1);
            }

            std::vector<VkPresentModeKHR> presentModes(presentModeCount);

            result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to query supported present modes\n");
                std::exit(1);
            }

            static constexpr std::pair<SwapchainPresentMode, VkPresentModeKHR> chain[] = {
                {SwapchainPresentMode::Immediate, VK_PRESENT_MODE_IMMEDIATE_KHR},
                {SwapchainPresentMode::Asynchronous, VK_PRESENT_MODE_MAILBOX_KHR},
                {SwapchainPresentMode::RelaxedSynchronous, VK_PRESENT_MODE_FIFO_RELAXED_KHR},
                {SwapchainPresentMode::Synchronous, VK_PRESENT_MODE_FIFO_KHR},
            };

            auto start = std::ranges::find(chain, config.presentMode, &std::pair<SwapchainPresentMode, VkPresentModeKHR>::first);

            for (auto it = start; it != std::end(chain); ++it) {
                if (std::ranges::find(presentModes, it->second) != presentModes.end()) {
                    return it->second;
                }
            }

            return VK_PRESENT_MODE_FIFO_KHR;
        }

        [[nodiscard]] VkSurfaceFormatKHR selectSurfaceFormat() {
            VkPhysicalDevice physicalDevice = device_->physicalDevice().handle();
            VkSurfaceKHR surface = surface_->handle();

            std::uint32_t surfaceFormatCount = 0;

            VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);

            if (result != VK_SUCCESS) {
                std::printf("error: failed to query supported surface formats\n");
                std::exit(1);
            }

            std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);

            result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormats.data());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to query supported surface formats\n");
                std::exit(1);
            }

            static constexpr VkSurfaceFormatKHR candidates[] = {
                {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
                {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR},
            };

            for (auto& candidate : candidates) {
                for (auto& format : surfaceFormats) {
                    if (format.format == candidate.format && format.colorSpace == candidate.colorSpace) {
                        return candidate;
                    }
                }
            }

            return surfaceFormats[0];
        }
    };
}