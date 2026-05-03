#pragma once

#include "../meta/resource.hpp"
#include "../resources/image.hpp"
#include "device.hpp"
#include "surface.hpp"

namespace lumina::render {
    /**
     * @brief The way that a swapchain will write to and swap framebuffers.
     *
     */
    enum class SwapchainPresentMode {
        /// @brief The swapchain will always wait for the next v-blank.
        Synchronous,

        /// @brief The swapchain waits for v-blank, but can submit late images if required.
        RelaxedSynchronous,

        /// @brief The swapchain overwrites the oldest image and presents the most recent at v-blank.
        Asynchronous,

        /// @brief The swapchain immediately presents the last processed image.
        Immediate,
    };

    /**
     * @brief Configuration for a swapchain.
     *
     */
    struct SwapchainConfig {
        SwapchainPresentMode presentMode;
        std::uint32_t minimumImageCount;
    };

    /**
     * @brief A system capable of displaying images directly to a surface.
     *
     * Note that this may not be available one very physical device. you must successfully create
     * a logical device that has the 'swapchains' feature enabled.
     */
    class Swapchain : public Resource<VkSwapchainKHR> {
        struct SurfaceCapabilities {
            std::uint32_t minimumImageCount;
            VkExtent2D extent;
            VkSurfaceTransformFlagBitsKHR transform;
            VkCompositeAlphaFlagBitsKHR compositeAlpha;
        };

    public:
        Swapchain() = default;

        /**
         * @brief Construct a new swapchain.
         *
         * @param device The swapchain-enabled device to operate with.
         * @param surface The surface to output to.
         * @param graphicsQueue The queue that will render to this swapchain's images.
         * @param presentQueue The queue that will present the images of the swapchain to the surface.
         * @param config The swapchain configuration to use.
         */
        Swapchain(Device& device, Surface& surface, Queue& graphicsQueue, Queue& presentQueue, const SwapchainConfig& config)
            : device_(&device), surface_(&surface) {
            SurfaceCapabilities capabilities = getSurfaceCapabilities(config);
            VkSurfaceFormatKHR surfaceFormat = selectSurfaceFormat();

            VkSwapchainCreateInfoKHR createInfo = {
                .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .pNext = nullptr,
                .flags = 0,
                .surface = *surface_,
                .minImageCount = capabilities.minimumImageCount,
                .imageFormat = surfaceFormat.format,
                .imageColorSpace = surfaceFormat.colorSpace,
                .imageExtent = capabilities.extent,
                .imageArrayLayers = 1,
                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .queueFamilyIndexCount = 0,
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
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2,
                createInfo.pQueueFamilyIndices = familyIndices.data();
            }

            VkResult result = vkCreateSwapchainKHR(*device_, &createInfo, nullptr, &resource());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to create swapchain\n");
                std::exit(1);
            }

            std::uint32_t swapchainImageCount = 0;

            result = vkGetSwapchainImagesKHR(*device_, resource(), &swapchainImageCount, nullptr);

            if (result != VK_SUCCESS) {
                std::printf("error: failed to retrieve swapchain images\n");
                std::exit(1);
            }

            std::vector<VkImage> images(swapchainImageCount);
            images_.reserve(swapchainImageCount);

            result = vkGetSwapchainImagesKHR(*device_, resource(), &swapchainImageCount, images.data());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to retrieve swapchain images\n");
                std::exit(1);
            }

            for (auto& image : images) {
                images_.emplace_back(image, ImageProvider::Swapchain);
            }
        }

        /**
         * @brief Destroy the swapchain.
         *
         */
        ~Swapchain() {
            destroy();
        }

        Swapchain(Swapchain&&) noexcept = default;

        Swapchain& operator=(Swapchain&&) noexcept = default;

        /**
         * @brief Construct a new swapchain.
         *
         * @param device The swapchain-enabled device to operate with.
         * @param surface The surface to output to.
         * @param graphicsQueue The queue that will render to this swapchain's images.
         * @param presentQueue The queue that will present the images of the swapchain to the surface.
         * @param config The swapchain configuration to use.
         */
        void create(Device& device, Surface& surface, Queue& graphicsQueue, Queue& presentQueue, const SwapchainConfig& config) {
            if (*this) {
                return;
            }

            device_ = &device;
            surface_ = &surface;

            SurfaceCapabilities capabilities = getSurfaceCapabilities(config);
            VkSurfaceFormatKHR surfaceFormat = selectSurfaceFormat();

            VkSwapchainCreateInfoKHR createInfo = {
                .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .pNext = nullptr,
                .flags = 0,
                .surface = *surface_,
                .minImageCount = capabilities.minimumImageCount,
                .imageFormat = surfaceFormat.format,
                .imageColorSpace = surfaceFormat.colorSpace,
                .imageExtent = capabilities.extent,
                .imageArrayLayers = 1,
                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .queueFamilyIndexCount = 0,
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
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2,
                createInfo.pQueueFamilyIndices = familyIndices.data();
            }

            VkResult result = vkCreateSwapchainKHR(*device_, &createInfo, nullptr, &resource());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to create swapchain\n");
                std::exit(1);
            }

            std::uint32_t swapchainImageCount = 0;

            result = vkGetSwapchainImagesKHR(*device_, resource(), &swapchainImageCount, nullptr);

            if (result != VK_SUCCESS) {
                std::printf("error: failed to retrieve swapchain images\n");
                std::exit(1);
            }

            std::vector<VkImage> images(swapchainImageCount);
            images_.reserve(swapchainImageCount);

            result = vkGetSwapchainImagesKHR(*device_, resource(), &swapchainImageCount, images.data());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to retrieve swapchain images\n");
                std::exit(1);
            }

            for (auto& image : images) {
                images_.emplace_back(image, ImageProvider::Swapchain);
            }
        }

        /**
         * @brief Destroy the swapchain.
         *
         */
        void destroy() {
            if (!*this) {
                return;
            }
            vkDestroySwapchainKHR(*device_, resource(), nullptr);

            surface_ = nullptr;
            device_ = nullptr;

            invalidate();
        }

        /**
         * @brief Access the swapchain's images.
         *
         * @return std::span<Image>
         */
        [[nodiscard]] std::span<Image> images() {
            return images_;
        }

        /**
         * @brief Access the swapchain's images.
         *
         * @return std::span<const Image>
         */
        [[nodiscard]] std::span<const Image> images() const {
            return images_;
        }

    private:
        Device* device_ = nullptr;
        Surface* surface_ = nullptr;
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
            VkPhysicalDevice physicalDevice = device_->physicalDevice();
            VkSurfaceKHR surface = *surface_;

            VkSurfaceCapabilitiesKHR surfaceCapabilities;

            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

            VkExtent2D extent = surfaceCapabilities.currentExtent;

            if (extent.width == UINT32_MAX || extent.height == UINT32_MAX) {
                extent.width = surface_->extent().width;
                extent.height = surface_->extent().height;
            }

            return {
                .minimumImageCount = std::clamp(config.minimumImageCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount),
                .extent = extent,
                .transform = surfaceCapabilities.currentTransform,
                .compositeAlpha = selectCompositeAlpha(surfaceCapabilities.supportedCompositeAlpha),
            };
        }

        [[nodiscard]] VkPresentModeKHR selectPresentMode(const SwapchainConfig& config) {
            VkPhysicalDevice physicalDevice = device_->physicalDevice();
            VkSurfaceKHR surface = *surface_;

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
            VkPhysicalDevice physicalDevice = device_->physicalDevice();
            VkSurfaceKHR surface = *surface_;

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