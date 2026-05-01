#pragma once

#include <lumina/render/instance.hpp>
#include <lumina/render/queue.hpp>
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

    class Device {
    public:
        Device(const DeviceInfo& info)
            : physicalDevice_(info.physicalDevice), presentQueue_(nullptr, {}) {
            Families families = getAvailableQueueFamilies(info);

            std::vector<std::uint32_t> familyUsage(families.size(), 0);

            auto graphicsAssignment = assignQueues(families, familyUsage, VK_QUEUE_GRAPHICS_BIT, info.features.graphics.count);
            auto computeAssignment = assignQueues(families, familyUsage, VK_QUEUE_COMPUTE_BIT, info.features.compute.count);
            auto transferAssignment = assignQueues(families, familyUsage, VK_QUEUE_TRANSFER_BIT, info.features.transfer.count);

            std::optional<QueueAssignment> presentAssignment;

            if (info.features.presentation) {
                if (!tryPresent(presentAssignment, info, graphicsAssignment) &&
                    !tryPresent(presentAssignment, info, computeAssignment) &&
                    !tryPresent(presentAssignment, info, transferAssignment)) {
                    std::printf("error: no assigned queue family supports presentation\n");
                    std::exit(1);
                }
            }

            if ((info.features.graphics.count > 0 && !graphicsAssignment) ||
                (info.features.compute.count > 0 && !computeAssignment) ||
                (info.features.transfer.count > 0 && !transferAssignment)) {
                std::printf("error: physical device does not satisfy queue requirements\n");
                std::exit(1);
            }

            QueueCountLookup familyQueueCount;

            registerFamily(familyQueueCount, graphicsAssignment, info.features.graphics.count);
            registerFamily(familyQueueCount, computeAssignment, info.features.compute.count);
            registerFamily(familyQueueCount, transferAssignment, info.features.transfer.count);

            if (info.features.presentation) {
                registerFamily(familyQueueCount, presentAssignment, 1);
            }

            std::vector<float> priorities(families.size(), 1.0f);

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

            Extensions extensions = getExtensions(info);
            Names selectedExtensions;
            bool swapchainSupported = false;

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

            VkResult result = vkCreateDevice(info.physicalDevice.handle(), &createInfo, nullptr, &device_);

            if (result != VK_SUCCESS) {
                std::printf("error: failed to create device\n");
                std::exit(1);
            }

            auto retrieveQueues = [&](const std::optional<QueueAssignment>& assignment, std::uint32_t count) -> std::vector<Queue> {
                std::vector<Queue> queues;

                queues.reserve(count);

                for (std::uint32_t i = 0; i < count; ++i) {
                    VkQueue queue;

                    vkGetDeviceQueue(device_, assignment->familyIndex, assignment->queueOffset + i, &queue);
                    queues.emplace_back(queue, assignment.value());
                }

                return queues;
            };

            graphicsQueues_ = retrieveQueues(graphicsAssignment, info.features.graphics.count);
            computeQueues_ = retrieveQueues(computeAssignment, info.features.compute.count);
            transferQueues_ = retrieveQueues(transferAssignment, info.features.transfer.count);

            if (info.features.presentation) {
                VkQueue queue;

                vkGetDeviceQueue(device_, presentAssignment->familyIndex, presentAssignment->queueOffset, &queue);

                presentQueue_ = Queue(queue, *presentAssignment);
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

        [[nodiscard]] PhysicalDevice physicalDevice() const {
            return physicalDevice_;
        }

        [[nodiscard]] std::vector<Queue> graphicsQueues() const {
            return graphicsQueues_;
        }

        [[nodiscard]] std::vector<Queue> computeQueues() const {
            return computeQueues_;
        }

        [[nodiscard]] std::vector<Queue> transferQueues() const {
            return transferQueues_;
        }

        [[nodiscard]] Queue presentQueue() const {
            return presentQueue_;
        }

    private:
        using Families = std::vector<VkQueueFamilyProperties>;
        using Extensions = std::vector<VkExtensionProperties>;
        using Names = std::vector<const char*>;
        using QueueCountLookup = std::unordered_map<std::uint32_t, std::uint32_t>;

        VkDevice device_ = nullptr;
        PhysicalDevice physicalDevice_;

        std::vector<Queue> graphicsQueues_;
        std::vector<Queue> computeQueues_;
        std::vector<Queue> transferQueues_;
        Queue presentQueue_;

        [[nodiscard]] Families getAvailableQueueFamilies(const DeviceInfo& info) {
            std::uint32_t familyCount = 0;

            vkGetPhysicalDeviceQueueFamilyProperties(info.physicalDevice.handle(), &familyCount, nullptr);

            Families families(familyCount);

            vkGetPhysicalDeviceQueueFamilyProperties(info.physicalDevice.handle(), &familyCount, families.data());

            return families;
        }

        [[nodiscard]] static std::optional<std::uint32_t> findFamily(const Families& families, std::vector<std::uint32_t>& familyUsed, VkQueueFlags required) {
            for (std::uint32_t i = 0; i < families.size(); ++i) {
                if ((families[i].queueFlags & required) == required && familyUsed[i] < families[i].queueCount) {
                    return i;
                }
            }

            return std::nullopt;
        };

        [[nodiscard]] static std::optional<QueueAssignment> assignQueues(const Families& families, std::vector<std::uint32_t>& familyUsed, VkQueueFlags required, std::uint32_t count) {
            if (count == 0) {
                return std::nullopt;
            }

            auto family = findFamily(families, familyUsed, required);

            if (!family) {
                return std::nullopt;
            }

            QueueAssignment assignment = {
                .familyIndex = family.value(),
                .queueOffset = familyUsed[family.value()],
            };

            familyUsed[family.value()] += count;

            return assignment;
        };

        [[nodiscard]] static bool tryPresent(std::optional<QueueAssignment>& writeback, const DeviceInfo& info, const std::optional<QueueAssignment>& assignment) {
            if (!assignment) {
                return false;
            }

            VkBool32 supported = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(info.physicalDevice.handle(), assignment->familyIndex, info.features.presentation->surface.handle(), &supported);

            if (supported) {
                writeback = assignment;

                return true;
            }

            return false;
        };

        void registerFamily(QueueCountLookup& lookup, const std::optional<QueueAssignment>& assignment, std::uint32_t count) {
            if (assignment) {
                lookup[assignment->familyIndex] = std::max(lookup[assignment->familyIndex], assignment->queueOffset + count);
            }
        };

        [[nodiscard]] Extensions getExtensions(const DeviceInfo& info) {
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

            return extensions;
        }
    };
}