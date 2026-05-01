#pragma once

#include <lumina/render/instance.hpp>
#include <lumina/render/surface.hpp>
#include <unordered_map>
#include <vulkan/vulkan.h>

namespace lumina::render {
    struct QueueRequirements {
        std::uint32_t count;
    };

    struct DevicePresentFeature {
        Surface& surface;
    };

    struct DeviceFeatures {
        QueueRequirements graphics;
        QueueRequirements compute;
        QueueRequirements transfer;
        DevicePresentFeature* presentation;
    };

    struct DeviceInfo {
        DeviceFeatures features;
        Instance& instance;
        PhysicalDevice& physicalDevice;
    };

    struct QueueAssignment {
        std::uint32_t familyIndex;
        std::uint32_t queueOffset;
    };

    class Device {
    public:
        Device(const DeviceInfo& info) {
            std::uint32_t familyCount = 0;

            vkGetPhysicalDeviceQueueFamilyProperties(info.physicalDevice.handle(), &familyCount, nullptr);

            std::vector<VkQueueFamilyProperties> families(familyCount);

            vkGetPhysicalDeviceQueueFamilyProperties(info.physicalDevice.handle(), &familyCount, families.data());

            std::vector<std::uint32_t> familyUsed(familyCount, 0);

            auto findFamily = [&](VkQueueFlags required) -> std::optional<std::uint32_t> {
                for (std::uint32_t i = 0; i < familyCount; ++i) {
                    if ((families[i].queueFlags & required) == required && familyUsed[i] < families[i].queueCount) {
                        return i;
                    }
                }

                return std::nullopt;
            };

            auto assignQueues = [&](VkQueueFlags required, std::uint32_t count) -> std::optional<QueueAssignment> {
                if (count == 0) {
                    return std::nullopt;
                }
                auto family = findFamily(required);

                if (!family) {
                    return std::nullopt;
                }

                QueueAssignment a{*family, familyUsed[*family]};
                familyUsed[*family] += count;
                return a;
            };

            auto graphicsAssign = assignQueues(VK_QUEUE_GRAPHICS_BIT, info.features.graphics.count);
            auto computeAssign = assignQueues(VK_QUEUE_COMPUTE_BIT, info.features.compute.count);
            auto transferAssign = assignQueues(VK_QUEUE_TRANSFER_BIT, info.features.transfer.count);

            std::optional<QueueAssignment> presentAssign;
            if (info.features.presentation) {
                auto& surf = info.features.presentation->surface;

                auto tryPresent = [&](const std::optional<QueueAssignment>& a) -> bool {
                    if (!a) {
                        return false;
                    }

                    VkBool32 supported = VK_FALSE;
                    vkGetPhysicalDeviceSurfaceSupportKHR(info.physicalDevice.handle(), a->familyIndex, surf.handle(), &supported);

                    if (supported) {
                        presentAssign = a;
                        return true;
                    }

                    return false;
                };

                if (!tryPresent(graphicsAssign) && !tryPresent(computeAssign) && !tryPresent(transferAssign)) {
                    std::printf("error: no assigned queue family supports presentation\n");
                    std::exit(1);
                }
            }

            if ((info.features.graphics.count > 0 && !graphicsAssign) ||
                (info.features.compute.count > 0 && !computeAssign) ||
                (info.features.transfer.count > 0 && !transferAssign)) {
                std::printf("error: physical device does not satisfy queue requirements\n");
                std::exit(1);
            }

            std::unordered_map<std::uint32_t, std::uint32_t> familyQueueCount;

            auto registerFamily = [&](const std::optional<QueueAssignment>& a, std::uint32_t count) {
                if (a) {
                    familyQueueCount[a->familyIndex] = std::max(familyQueueCount[a->familyIndex], a->queueOffset + count);
                }
            };

            registerFamily(graphicsAssign, info.features.graphics.count);
            registerFamily(computeAssign, info.features.compute.count);
            registerFamily(transferAssign, info.features.transfer.count);

            if (info.features.presentation) {
                registerFamily(presentAssign, 1);
            }

            std::vector<float> priorities(familyCount, 1.0f);

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

            for (auto& [familyIdx, count] : familyQueueCount) {
                queueCreateInfos.push_back({
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .queueFamilyIndex = familyIdx,
                    .queueCount = count,
                    .pQueuePriorities = priorities.data(),
                });
            }

            std::uint32_t extensionCount = 0;

            VkResult result = vkEnumerateDeviceExtensionProperties(info.physicalDevice.handle(), nullptr, &extensionCount, nullptr);

            if (result != VK_SUCCESS) {
                std::printf("error: failed to enumerate device extensions\n");
                std::exit(1);
            }

            std::vector<VkExtensionProperties> extensions(extensionCount);

            result = vkEnumerateDeviceExtensionProperties(info.physicalDevice.handle(), nullptr, &extensionCount, extensions.data());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to enumerate device extensions\n");
                std::exit(1);
            }

            bool swapchainSupported = false;

            std::vector<const char*> selectedExtensions;

            constexpr const char* Swapchain = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

            for (auto& extension : extensions) {
                if (std::string_view(extension.extensionName) == Swapchain) {
                    swapchainSupported = true;
                    selectedExtensions.emplace_back(extension.extensionName);
                }
            }

            if (!swapchainSupported && info.features.presentation) {
                std::printf("error: presentation is required but is unsupported by the selected physical device\n");
                std::exit(1);
            }

            VkDeviceCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.size()),
                .pQueueCreateInfos = queueCreateInfos.data(),
                .enabledLayerCount = 0,
                .ppEnabledLayerNames = nullptr,
                .enabledExtensionCount = static_cast<std::uint32_t>(selectedExtensions.size()),
                .ppEnabledExtensionNames = selectedExtensions.data(),
                .pEnabledFeatures = nullptr,
            };

            result = vkCreateDevice(info.physicalDevice.handle(), &createInfo, nullptr, &device_);

            if (result != VK_SUCCESS) {
                std::printf("error: failed to create device\n");
                std::exit(1);
            }

            auto retrieveQueues = [&](const std::optional<QueueAssignment>& assignment, std::uint32_t count) -> std::vector<VkQueue> {
                std::vector<VkQueue> queues(count);

                for (std::uint32_t i = 0; i < count; ++i) {
                    vkGetDeviceQueue(device_, assignment->familyIndex, assignment->queueOffset + i, &queues[i]);
                }

                return queues;
            };

            graphicsQueues_ = retrieveQueues(graphicsAssign, info.features.graphics.count);
            computeQueues_ = retrieveQueues(computeAssign, info.features.compute.count);
            transferQueues_ = retrieveQueues(transferAssign, info.features.transfer.count);

            if (info.features.presentation) {
                vkGetDeviceQueue(device_, presentAssign->familyIndex, presentAssign->queueOffset, &presentQueue_);
            }
        }

        ~Device() {
            if (device_) {
                vkDestroyDevice(device_, nullptr);
                device_ = nullptr;
            }
        }

        [[nodiscard]] VkDevice handle() const {
            return device_;
        }

    private:
        VkDevice device_ = nullptr;

        std::vector<VkQueue> graphicsQueues_;
        std::vector<VkQueue> computeQueues_;
        std::vector<VkQueue> transferQueues_;

        VkQueue presentQueue_;
    };
}