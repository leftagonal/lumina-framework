#pragma once

#include "device.hpp"
#include "instance.hpp"
#include "presenter.hpp"
#include "vulkan.hpp"

#include <lumina/meta/console.hpp>
#include <lumina/meta/exceptions.hpp>

#include <unordered_map>

namespace lumina::renderer::subsystems {
    inline void Device::connect(Presenter& presenter, const DeviceRequirements& requirements) {
        meta::assert(instance_ == nullptr, "device has already been created");
        meta::assert(presenter.count() > 0, "minimum one window is required for device creation");
        meta::assert(!presenter.bound(), "provided presenter is already in-use by a different device");

        debugging_ = presenter.instance().debugging();
        instance_ = &presenter.instance();

        PhysicalDevices physicalDevices = getAvailablePhysicalDevices();

        pickMostSuitablePhysicalDevice(physicalDevices, requirements);

        QueueFamilies queueFamilies = getAvailableQueueFamilies();
        QueueFamilySelections queueSelections = pickSuitableQueueFamilies(queueFamilies, presenter);
        QueueCreateInfos queueCreateInfos = makeQueueCreateInfos(queueSelections);

        Names requiredExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };

        ExtensionProperties extensionProperties = getAvailableExtensions();

        tryEnableCompatibility(requiredExtensions, extensionProperties);

        meta::assert(
            testRequirements(requiredExtensions, extensionProperties),
            "one or more required Vulkan device extensions are unavailable");

        VkDeviceCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.createInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.createInfos.data(),
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<std::uint32_t>(requiredExtensions.size()),
            .ppEnabledExtensionNames = requiredExtensions.data(),
            .pEnabledFeatures = nullptr,
        };

        VkResult result = vkCreateDevice(physicalDevice_, &createInfo, nullptr, &device_);

        meta::assert(result == VK_SUCCESS, "logical device creation failed: {}", Vulkan_errorString(result));
        meta::logDebug(debugging_, "Vulkan logical device initialised");

        vkGetDeviceQueue(device_, queueSelections[0].familyIndex, queueSelections[0].queueIndex, &presentQueue_);
        vkGetDeviceQueue(device_, queueSelections[1].familyIndex, queueSelections[1].queueIndex, &graphicsQueue_);
        vkGetDeviceQueue(device_, queueSelections[2].familyIndex, queueSelections[2].queueIndex, &computeQueue_);
        vkGetDeviceQueue(device_, queueSelections[3].familyIndex, queueSelections[3].queueIndex, &transferQueue_);

        meta::logDebug(debugging_, "Vulkan queues assigned");
    }

    inline void Device::disconnect() {
        if (device_ != nullptr) {
            vkDestroyDevice(device_, nullptr);

            device_ = nullptr;
            presentQueue_ = nullptr;
            graphicsQueue_ = nullptr;
            computeQueue_ = nullptr;
            transferQueue_ = nullptr;
            instance_ = nullptr;

            meta::logDebug(debugging_, "Vulkan logical device destroyed");
        }
    }

    inline Device::~Device() {
        disconnect();
    }

    inline Device::PhysicalDevices Device::getAvailablePhysicalDevices() const {
        VkInstance instance = accessors::InstanceAccessor::instance(*instance_);

        std::uint32_t physicalDeviceCount = 0;

        VkResult result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

        meta::assert(result == VK_SUCCESS, "failed to enumerate Vulkan physical devices: {}", Vulkan_errorString(result));

        std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);

        result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

        meta::assert(result == VK_SUCCESS, "failed to enumerate Vulkan physical devices: {}", Vulkan_errorString(result));

        return physicalDevices;
    }

    inline Device::QueueFamilies Device::getAvailableQueueFamilies() const {
        std::uint32_t queueFamilyCount = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, nullptr);

        QueueFamilies queueFamilies(queueFamilyCount);

        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, queueFamilies.data());

        return queueFamilies;
    }

    inline void Device::pickMostSuitablePhysicalDevice(const PhysicalDevices& physicalDevices, const DeviceRequirements&) {
        if (physicalDevices.empty()) {
            throw meta::Exception("no supported Vulkan physical devices are available");
        }

        physicalDevice_ = physicalDevices.front();

        VkPhysicalDeviceProperties properties = {};

        vkGetPhysicalDeviceProperties(physicalDevice_, &properties);

        meta::logDebug(debugging_, "Vulkan physical device selected:");

        meta::logDebugListElement(debugging_, 1, "name: {}", properties.deviceName);
        meta::logDebugListElement(debugging_, 1, "device ID: {}", properties.deviceID);
        meta::logDebugListElement(debugging_, 1, "vendor ID: {}", properties.vendorID);
    }

    inline Device::QueueFamilySelections Device::pickSuitableQueueFamilies(const QueueFamilies& families, Presenter& presenter) const {
        // we probably shouldn't assume index 0, but oh well, it'll work
        VkSurfaceKHR drivingSurface = accessors::PresenterAccessor::surface(presenter, 0);

        std::optional<QueueFamilySelection> graphics;
        std::optional<QueueFamilySelection> present;
        std::optional<QueueFamilySelection> compute;
        std::optional<QueueFamilySelection> transfer;

        for (std::uint32_t i = 0; i < families.size(); ++i) {
            auto& family = families[i];
            VkBool32 supported = VK_FALSE;
            VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice_, i, drivingSurface, &supported);

            meta::assert(result == VK_SUCCESS, "failed to query physical device's presentation support");

            bool isPresentCapable = supported == VK_TRUE;
            bool isGraphicsCapable = family.queueFlags & VK_QUEUE_GRAPHICS_BIT;

            if (isPresentCapable) {
                present = QueueFamilySelection{
                    .familyIndex = i,
                    .queueIndex = 0,
                };
            }

            if (isGraphicsCapable) {
                graphics = QueueFamilySelection{
                    .familyIndex = i,
                    .queueIndex = 0,
                };
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

            if (isComputeCapable && !isGraphicsCapable && !isTransferCapable) {
                compute = QueueFamilySelection{
                    i,
                    0,
                };
            }

            if (isComputeCapable && !isGraphicsCapable && !compute) {
                compute = QueueFamilySelection{
                    i,
                    0,
                };
            }

            if (isTransferCapable && !isGraphicsCapable && !isComputeCapable) {
                transfer = QueueFamilySelection{
                    i,
                    0,
                };
            }

            if (isTransferCapable && !isGraphicsCapable && !transfer) {
                transfer = QueueFamilySelection{
                    i,
                    0,
                };
            }
        }

        if (!compute) {
            meta::logDebug(debugging_, "Vulkan compute queue falling back to graphics queue");

            compute = graphics;
        }

        if (!transfer) {
            meta::logDebug(debugging_, "Vulkan transfer queue falling back to graphics queue");

            transfer = graphics;
        }

        meta::logDebug(debugging_, "Vulkan queues have been identified:");

        meta::logDebugListElement(debugging_, 1, "present: family {}, index {}", present->familyIndex, present->queueIndex);
        meta::logDebugListElement(debugging_, 1, "graphics: family {}, index {}", graphics->familyIndex, graphics->queueIndex);
        meta::logDebugListElement(debugging_, 1, "compute: family {}, index {}", compute->familyIndex, compute->queueIndex);
        meta::logDebugListElement(debugging_, 1, "transfer: family {}, index {}", transfer->familyIndex, transfer->queueIndex);

        return QueueFamilySelections{
            present.value(),
            graphics.value(),
            compute.value(),
            transfer.value(),
        };
    }

    inline Device::QueueCreateInfos Device::makeQueueCreateInfos(const QueueFamilySelections& queueSelections) const {
        QueueCreateInfos queueCreateInfos;

        std::unordered_map<std::uint32_t, VkDeviceQueueCreateInfo> queueCreateInfoMap;

        for (auto& selection : queueSelections) {
            if (queueCreateInfoMap.contains(selection.familyIndex)) {
                continue;
            }

            queueCreateInfoMap[selection.familyIndex] = VkDeviceQueueCreateInfo{
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

    inline Device::ExtensionProperties Device::getAvailableExtensions() const {
        std::uint32_t extensionCount = 0;

        VkResult result = vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &extensionCount, nullptr);

        meta::assert(result == VK_SUCCESS, "failed to enumerate available Vulkan physical device extensions: {}", Vulkan_errorString(result));

        ExtensionProperties extensions(extensionCount);

        result = vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &extensionCount, extensions.data());

        meta::assert(result == VK_SUCCESS, "failed to enumerate available Vulkan physical device extensions: {}", Vulkan_errorString(result));

        return extensions;
    }

    inline void Device::tryEnableCompatibility(Names& requirements, const ExtensionProperties& available) const {
        for (auto& extension : available) {
            if (std::string_view(extension.extensionName) == "VK_KHR_portability_subset") {
                requirements.emplace_back(extension.extensionName);

                break;
            }
        }
    }

    inline bool Device::testRequirements(const Names& requirements, const ExtensionProperties& available) const {
        bool succeeded = true;

        meta::logDebug(debugging_, "required Vulkan device extensions: {}", requirements.size());

        for (auto& requirement : requirements) {
            bool found = false;

            for (auto& candidate : available) {
                if (std::string_view(requirement) == candidate.extensionName) {
                    found = true;

                    meta::logDebugListElement(debugging_, 1, "{}", requirement);

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