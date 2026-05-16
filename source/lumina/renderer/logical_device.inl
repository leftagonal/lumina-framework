#pragma once

#include <lumina/meta/exceptions.hpp>

#include "instance.hpp"
#include "logical_device.hpp"

#include <unordered_map>

namespace lumina::renderer {
    inline LogicalDevice::LogicalDevice(Instance& instance, SurfaceManager& surfaceManager, const DeviceRequirements& requirements) {
        create(instance, surfaceManager, requirements);
    }

    inline LogicalDevice::~LogicalDevice() {
        destroy();
    }

    LogicalDevice::LogicalDevice(LogicalDevice&& other) noexcept
        : instance_(other.instance_), physicalDevice_(other.physicalDevice_),
          device_(other.device_), presentQueues_(std::move(other.presentQueues_)), graphicsQueues_(std::move(other.graphicsQueues_)),
          computeQueues_(std::move(other.computeQueues_)), transferQueues_(std::move(other.transferQueues_)) {
        other.instance_ = nullptr;
        other.physicalDevice_ = nullptr;
        other.device_ = nullptr;

        other.presentQueues_.clear();
        other.graphicsQueues_.clear();
        other.computeQueues_.clear();
        other.transferQueues_.clear();
    }

    LogicalDevice& LogicalDevice::operator=(LogicalDevice&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        instance_ = other.instance_;
        physicalDevice_ = other.physicalDevice_;
        device_ = other.device_;

        presentQueues_ = std::move(other.presentQueues_);
        graphicsQueues_ = std::move(other.graphicsQueues_);
        computeQueues_ = std::move(other.computeQueues_);
        transferQueues_ = std::move(other.transferQueues_);

        other.instance_ = nullptr;
        other.physicalDevice_ = nullptr;
        other.device_ = nullptr;

        other.presentQueues_.clear();
        other.graphicsQueues_.clear();
        other.computeQueues_.clear();
        other.transferQueues_.clear();

        return *this;
    }

    inline void LogicalDevice::create(Instance& instance, SurfaceManager& surfaceManager, const DeviceRequirements& requirements) {
        if (valid()) {
            return;
        }

        meta::assert(!surfaceManager.empty(), "logical device requires at least one valid surface before creation");

        instance_ = &instance;

        PhysicalDevices physicalDevices = getAvailablePhysicalDevices();

        pickMostSuitablePhysicalDevice(physicalDevices, requirements);

        QueueFamilies queueFamilies = getAvailableQueueFamilies();
        QueueSelections queueSelections = pickSuitableQueueFamilies(queueFamilies, surfaceManager);
        QueueCreateInfos queueCreateInfos = makeQueueCreateInfos(queueSelections);

        Names requiredExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };

        ExtensionProperties extensionProperties = getAvailableExtensions();

        tryEnableCompatibility(requiredExtensions, extensionProperties);

        bool requirementsMet = testRequirements(requiredExtensions, extensionProperties);

        meta::assert(requirementsMet, "one or more required Vulkan device extensions are unavailable");

        std::uint32_t queueInfoCount = static_cast<std::uint32_t>(queueCreateInfos.createInfos.size());
        std::uint32_t extensionCount = static_cast<std::uint32_t>(requiredExtensions.size());

        VkDeviceCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueCreateInfoCount = queueInfoCount,
            .pQueueCreateInfos = queueCreateInfos.createInfos.data(),
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = extensionCount,
            .ppEnabledExtensionNames = requiredExtensions.data(),
            .pEnabledFeatures = nullptr,
        };

        VkResult result = vkCreateDevice(physicalDevice_, &createInfo, nullptr, &device_);

        meta::assert(result == VK_SUCCESS, "logical device creation failed: {}", core::Vulkan_errorString(result));
        meta::logDebug(validation(), "Vulkan logical device initialised");

        for (auto& selection : queueSelections) {
            switch (selection.intent) {
                case QueueIntent::Present: {
                    presentQueues_.emplace_back(selection.entry);

                    break;
                }
                case QueueIntent::Graphics: {
                    graphicsQueues_.emplace_back(selection.entry);

                    break;
                }
                case QueueIntent::Compute: {
                    computeQueues_.emplace_back(selection.entry);

                    break;
                }
                case QueueIntent::Transfer: {
                    transferQueues_.emplace_back(selection.entry);

                    break;
                }
            }
        }
    }

    inline Instance& LogicalDevice::instance() {
        return *instance_;
    }

    inline const Instance& LogicalDevice::instance() const {
        return *instance_;
    }

    inline void LogicalDevice::destroy() {
        if (device_ == nullptr) {
            return;
        }

        vkDestroyDevice(device_, nullptr);

        meta::logDebug(validation(), "Vulkan logical device destroyed");

        instance_ = nullptr;
        physicalDevice_ = nullptr;
        device_ = nullptr;

        presentQueues_.clear();
        graphicsQueues_.clear();
        computeQueues_.clear();
        transferQueues_.clear();
    }

    inline bool LogicalDevice::valid() const {
        return device_ != nullptr;
    }

    inline LogicalDevice::operator bool() const {
        return valid();
    }

    inline bool LogicalDevice::validation() const {
        return instance_->applicationInfo().features.validation;
    }

    inline LogicalDevice::PhysicalDevices LogicalDevice::getAvailablePhysicalDevices() const {
        VkInstance instance = accessors::InstanceAccessor::instance(*instance_);

        std::uint32_t physicalDeviceCount = 0;

        VkResult result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

        meta::assert(result == VK_SUCCESS, "failed to enumerate Vulkan physical devices: {}", core::Vulkan_errorString(result));

        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);

        result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

        meta::assert(result == VK_SUCCESS, "failed to enumerate Vulkan physical devices: {}", core::Vulkan_errorString(result));

        return physicalDevices;
    }

    inline LogicalDevice::QueueFamilies LogicalDevice::getAvailableQueueFamilies() const {
        std::uint32_t queueFamilyCount = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, nullptr);

        QueueFamilies queueFamilies(queueFamilyCount);

        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, queueFamilies.data());

        return queueFamilies;
    }

    inline void LogicalDevice::pickMostSuitablePhysicalDevice(const PhysicalDevices& physicalDevices, const DeviceRequirements&) {
        meta::assert(!physicalDevices.empty(), "no supported Vulkan physical devices are available");

        physicalDevice_ = physicalDevices.front();

        VkPhysicalDeviceProperties properties = {};

        vkGetPhysicalDeviceProperties(physicalDevice_, &properties);

        meta::logDebug(validation(), "Vulkan physical device selected:");
        meta::logDebugListElement(validation(), 1, "name: {}", properties.deviceName);
        meta::logDebugListElement(validation(), 1, "device ID: {}", properties.deviceID);
        meta::logDebugListElement(validation(), 1, "vendor ID: {}", properties.vendorID);
    }

    inline auto LogicalDevice::pickSuitableQueueFamilies(const QueueFamilies& families, SurfaceManager& surfaceManager) const -> QueueSelections {
        VkSurfaceKHR drivingSurface = accessors::SurfaceManagerAccessor::firstValidSurface(surfaceManager);

        std::optional<QueueFamilySelection> graphics;
        std::optional<QueueFamilySelection> present;
        std::optional<QueueFamilySelection> compute;
        std::optional<QueueFamilySelection> transfer;

        for (std::uint32_t i = 0; i < families.size(); ++i) {
            auto& family = families[i];

            VkBool32 presentSupported = VK_FALSE;
            VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice_, i, drivingSurface, &presentSupported);

            meta::assert(result == VK_SUCCESS, "failed to query physical device's presentation support");

            bool isPresentCapable = presentSupported == VK_TRUE;
            bool isGraphicsCapable = family.queueFlags & VK_QUEUE_GRAPHICS_BIT;

            if (isPresentCapable) {
                present = {i, 0};
            }

            if (isGraphicsCapable) {
                graphics = {i, 0};
            }

            if (graphics && present) {
                break;
            }
        }

        meta::assert(present.has_value(), "physical device does not expose any present-capable queues");
        meta::assert(graphics.has_value(), "physical device does not expose any graphics-capable queues");

        // identify and prefer dedicated families
        for (std::uint32_t i = 0; i < families.size(); ++i) {
            auto& family = families[i];

            bool isGraphicsCapable = family.queueFlags & VK_QUEUE_GRAPHICS_BIT;
            bool isComputeCapable = family.queueFlags & VK_QUEUE_COMPUTE_BIT;
            bool isTransferCapable = family.queueFlags & VK_QUEUE_TRANSFER_BIT;

            bool isUniqueCompute = isComputeCapable && !isGraphicsCapable && !isTransferCapable;
            bool isSharedCompute = isComputeCapable && !isGraphicsCapable && !compute;

            if (isUniqueCompute || isSharedCompute) {
                compute = {i, 0};
            }

            bool isUniqueTransfer = isTransferCapable && !isGraphicsCapable && !isComputeCapable;
            bool isSharedTransfer = isTransferCapable && !isGraphicsCapable && !transfer;

            if (isUniqueTransfer || isSharedTransfer) {
                transfer = {i, 0};
            }
        }

        bool computeFallback = false;

        if (!compute) {
            meta::logDebug(validation(), "Vulkan compute queue falling back to graphics queue");

            compute = graphics;
            computeFallback = true;
        }

        bool transferFallback = false;

        if (!transfer) {
            meta::logDebug(validation(), "Vulkan transfer queue falling back to graphics queue");

            transfer = graphics;
            transferFallback = true;
        }

        meta::logDebug(validation(), "Vulkan queues have been identified:");
        meta::logDebugListElement(validation(), 1, "present: family {}, index {}", present->familyIndex, present->queueIndex);
        meta::logDebugListElement(validation(), 1, "graphics: family {}, index {}", graphics->familyIndex, graphics->queueIndex);
        meta::logDebugListElement(validation(), 1, "compute: family {}, index {}", compute->familyIndex, compute->queueIndex);
        meta::logDebugListElement(validation(), 1, "transfer: family {}, index {}", transfer->familyIndex, transfer->queueIndex);

        return {
            QueueSelection{
                .entry = {
                    .selection = present.value(),
                    .definition = {
                        .familyIndex = present->familyIndex,
                        .queueCount = families[present->familyIndex].queueCount,
                    },
                },
                .intent = QueueIntent::Present,
                .fallback = true,
            },
            QueueSelection{
                .entry = {
                    .selection = graphics.value(),
                    .definition = {
                        .familyIndex = graphics->familyIndex,
                        .queueCount = families[graphics->familyIndex].queueCount,
                    },
                },
                .intent = QueueIntent::Graphics,
                .fallback = true,
            },
            QueueSelection{
                .entry = {
                    .selection = compute.value(),
                    .definition = {
                        .familyIndex = compute->familyIndex,
                        .queueCount = families[compute->familyIndex].queueCount,
                    },
                },
                .intent = QueueIntent::Compute,
                .fallback = computeFallback,
            },
            QueueSelection{
                .entry = {
                    .selection = transfer.value(),
                    .definition = {
                        .familyIndex = transfer->familyIndex,
                        .queueCount = families[transfer->familyIndex].queueCount,
                    },
                },
                .intent = QueueIntent::Transfer,
                .fallback = transferFallback,
            },
        };
    }

    inline LogicalDevice::QueueCreateInfos LogicalDevice::makeQueueCreateInfos(const QueueSelections& queueSelections) const {
        QueueCreateInfos queueCreateInfos;

        std::unordered_map<std::uint32_t, VkDeviceQueueCreateInfo> queueCreateInfoMap;

        for (auto& selection : queueSelections) {
            if (queueCreateInfoMap.contains(selection.entry.definition.familyIndex)) {
                // Check if this queue is actually independent
                // If so, we must increase the number of queues in this family
                if (!selection.fallback) {
                    ++queueCreateInfoMap[selection.entry.definition.familyIndex].queueCount;
                }

                continue;
            }

            queueCreateInfoMap[selection.entry.definition.familyIndex] = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = selection.entry.definition.familyIndex,
                .queueCount = 1,
                .pQueuePriorities = nullptr,
            };
        }

        for (auto& [familyIndex, queueCreateInfo] : queueCreateInfoMap) {
            auto& priorities = queueCreateInfos.queuePriorities.emplace_back(queueCreateInfo.queueCount, 1.0);

            queueCreateInfo.pQueuePriorities = priorities.data();
            queueCreateInfos.createInfos.emplace_back(queueCreateInfo);
        }

        return queueCreateInfos;
    }

    inline LogicalDevice::ExtensionProperties LogicalDevice::getAvailableExtensions() const {
        std::uint32_t extensionCount = 0;

        VkResult result = vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &extensionCount, nullptr);

        meta::assert(result == VK_SUCCESS, "failed to enumerate available Vulkan physical device extensions: {}", core::Vulkan_errorString(result));

        ExtensionProperties extensions(extensionCount);

        result = vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &extensionCount, extensions.data());

        meta::assert(result == VK_SUCCESS, "failed to enumerate available Vulkan physical device extensions: {}", core::Vulkan_errorString(result));

        return extensions;
    }

    inline void LogicalDevice::tryEnableCompatibility(Names& requirements, const ExtensionProperties& available) const {
        for (auto& extension : available) {
            if (std::string_view(extension.extensionName) != "VK_KHR_portability_subset") {
                continue;
            }

            requirements.emplace_back(extension.extensionName);

            return;
        }
    }

    inline bool LogicalDevice::testRequirements(const Names& requirements, const ExtensionProperties& available) const {
        bool succeeded = true;

        meta::logDebug(validation(), "required Vulkan device extensions: {}", requirements.size());

        for (auto& requirement : requirements) {
            bool found = false;

            for (auto& candidate : available) {
                if (std::string_view(requirement) == candidate.extensionName) {
                    found = true;

                    meta::logDebugListElement(validation(), 1, "{}", requirement);

                    break;
                }
            }

            if (!found) {
                meta::logError("Vulkan device extension is required but unsupported: {}", requirement);

                succeeded = false;
            }
        }

        return succeeded;
    }
}
