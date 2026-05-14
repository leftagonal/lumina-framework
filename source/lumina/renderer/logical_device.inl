#pragma once

#include <lumina/meta/console.hpp>
#include <lumina/meta/exceptions.hpp>

#include "instance.hpp"
#include "logical_device.hpp"
#include "vulkan.hpp"
#include "window_manager.hpp"

#include <unordered_map>

namespace lumina::renderer {
    inline LogicalDevice::LogicalDevice(WindowManager& windowManager, const DeviceRequirements& requirements)
        : instance_(&windowManager.instance()), windowManager_(&windowManager) {
        meta::assert(windowManager.count() > 0, "minimum one window is required for device creation");

        PhysicalDevices physicalDevices = getAvailablePhysicalDevices();

        pickMostSuitablePhysicalDevice(physicalDevices, requirements);

        QueueFamilies queueFamilies = getAvailableQueueFamilies();
        QueueFamilySelections queueSelections = pickSuitableQueueFamilies(queueFamilies);
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

        meta::assert(result == VK_SUCCESS, "logical device creation failed: {}", Vulkan_errorString(result));
        meta::logDebug(validation(), "Vulkan logical device initialised");

        vkGetDeviceQueue(device_, queueSelections[0].familyIndex, queueSelections[0].queueIndex, &presentQueue_);
        vkGetDeviceQueue(device_, queueSelections[1].familyIndex, queueSelections[1].queueIndex, &graphicsQueue_);
        vkGetDeviceQueue(device_, queueSelections[2].familyIndex, queueSelections[2].queueIndex, &computeQueue_);
        vkGetDeviceQueue(device_, queueSelections[3].familyIndex, queueSelections[3].queueIndex, &transferQueue_);

        meta::logDebug(validation(), "Vulkan queues assigned");
    }

    inline LogicalDevice::~LogicalDevice() {
        destroy();
    }

    LogicalDevice::LogicalDevice(LogicalDevice&& other) noexcept
        : instance_(other.instance_), windowManager_(other.windowManager_), physicalDevice_(other.physicalDevice_),
          device_(other.device_), presentQueue_(other.presentQueue_), graphicsQueue_(other.graphicsQueue_),
          computeQueue_(other.computeQueue_), transferQueue_(other.transferQueue_) {
        other.instance_ = nullptr;
        other.windowManager_ = nullptr;
        other.physicalDevice_ = nullptr;
        other.device_ = nullptr;
        other.presentQueue_ = nullptr;
        other.graphicsQueue_ = nullptr;
        other.computeQueue_ = nullptr;
        other.transferQueue_ = nullptr;
    }

    LogicalDevice& LogicalDevice::operator=(LogicalDevice&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        instance_ = other.instance_;
        windowManager_ = other.windowManager_;
        physicalDevice_ = other.physicalDevice_;
        device_ = other.device_;
        presentQueue_ = other.presentQueue_;
        graphicsQueue_ = other.graphicsQueue_;
        computeQueue_ = other.computeQueue_;
        transferQueue_ = other.transferQueue_;

        other.instance_ = nullptr;
        other.windowManager_ = nullptr;
        other.physicalDevice_ = nullptr;
        other.device_ = nullptr;
        other.presentQueue_ = nullptr;
        other.graphicsQueue_ = nullptr;
        other.computeQueue_ = nullptr;
        other.transferQueue_ = nullptr;

        return *this;
    }

    inline Instance& LogicalDevice::instance() {
        return *instance_;
    }

    inline const Instance& LogicalDevice::instance() const {
        return *instance_;
    }

    inline WindowManager& LogicalDevice::windowManager() {
        return *windowManager_;
    }

    inline const WindowManager& LogicalDevice::windowManager() const {
        return *windowManager_;
    }

    inline void LogicalDevice::destroy() {
        if (device_ == nullptr) {
            return;
        }

        vkDestroyDevice(device_, nullptr);

        meta::logDebug(validation(), "Vulkan logical device destroyed");

        instance_ = nullptr;
        windowManager_ = nullptr;
        physicalDevice_ = nullptr;
        device_ = nullptr;
        presentQueue_ = nullptr;
        graphicsQueue_ = nullptr;
        computeQueue_ = nullptr;
        transferQueue_ = nullptr;
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

        meta::assert(result == VK_SUCCESS, "failed to enumerate Vulkan physical devices: {}", Vulkan_errorString(result));

        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);

        result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

        meta::assert(result == VK_SUCCESS, "failed to enumerate Vulkan physical devices: {}", Vulkan_errorString(result));

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

    inline LogicalDevice::QueueFamilySelections LogicalDevice::pickSuitableQueueFamilies(const QueueFamilies& families) const {
        Window& drivingWindow = windowManager_->windows().front();
        VkSurfaceKHR drivingSurface = accessors::WindowAccessor::surface(drivingWindow);

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

        if (!compute) {
            meta::logDebug(validation(), "Vulkan compute queue falling back to graphics queue");

            compute = graphics;
        }

        if (!transfer) {
            meta::logDebug(validation(), "Vulkan transfer queue falling back to graphics queue");

            transfer = graphics;
        }

        meta::logDebug(validation(), "Vulkan queues have been identified:");
        meta::logDebugListElement(validation(), 1, "present: family {}, index {}", present->familyIndex, present->queueIndex);
        meta::logDebugListElement(validation(), 1, "graphics: family {}, index {}", graphics->familyIndex, graphics->queueIndex);
        meta::logDebugListElement(validation(), 1, "compute: family {}, index {}", compute->familyIndex, compute->queueIndex);
        meta::logDebugListElement(validation(), 1, "transfer: family {}, index {}", transfer->familyIndex, transfer->queueIndex);

        return {
            present.value(),
            graphics.value(),
            compute.value(),
            transfer.value(),
        };
    }

    inline LogicalDevice::QueueCreateInfos LogicalDevice::makeQueueCreateInfos(const QueueFamilySelections& queueSelections) const {
        QueueCreateInfos queueCreateInfos;

        std::unordered_map<std::uint32_t, VkDeviceQueueCreateInfo> queueCreateInfoMap;

        for (auto& selection : queueSelections) {
            if (queueCreateInfoMap.contains(selection.familyIndex)) {
                continue;
            }

            queueCreateInfoMap[selection.familyIndex] = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = selection.familyIndex,
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

        meta::assert(result == VK_SUCCESS, "failed to enumerate available Vulkan physical device extensions: {}", Vulkan_errorString(result));

        ExtensionProperties extensions(extensionCount);

        result = vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &extensionCount, extensions.data());

        meta::assert(result == VK_SUCCESS, "failed to enumerate available Vulkan physical device extensions: {}", Vulkan_errorString(result));

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
