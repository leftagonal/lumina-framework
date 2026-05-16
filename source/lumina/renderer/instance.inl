#pragma once

#include <lumina/core/glfw.hpp>
#include <lumina/framework/information.hpp>
#include <lumina/meta/console.hpp>
#include <lumina/meta/exceptions.hpp>

#include "instance.hpp"

namespace lumina::renderer {
    inline Instance::Instance(const core::ApplicationInfo& info) {
        create(info);
    }

    inline Instance::~Instance() {
        destroy();
    }

    inline Instance::Instance(Instance&& other) noexcept
        : applicationInfo_(other.applicationInfo_), instance_(other.instance_) {
        other.applicationInfo_ = {};
        other.instance_ = nullptr;
    }

    inline Instance& Instance::operator=(Instance&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        applicationInfo_ = other.applicationInfo_;
        instance_ = other.instance_;

        other.applicationInfo_ = {};
        other.instance_ = nullptr;

        return *this;
    }

    inline Instance::operator bool() const {
        return valid();
    }

    inline void Instance::create(const core::ApplicationInfo& info) {
        if (valid()) {
            return;
        }

        applicationInfo_ = info;

        std::uint32_t driverVersion = getDriverSupportedVersion();
        std::uint32_t driverVersionMajor = VK_API_VERSION_MAJOR(driverVersion);
        std::uint32_t driverVersionMinor = VK_API_VERSION_MINOR(driverVersion);
        std::uint32_t driverVariant = VK_API_VERSION_VARIANT(driverVersion);

        bool vulkan13 = driverVersionMajor >= 1 && driverVersionMinor >= 3;

        meta::assert(driverVariant == 0, "non-standard Vulkan variant detected ({}), cannot proceed", driverVariant);
        meta::assert(vulkan13, "Vulkan 1.3 requirements, system supports {}.{}", driverVersionMajor, driverVersionMinor);

        ExtensionProperties availableExtensions = getAvailableExtensions();
        LayerProperties availableLayers = getAvailableLayers();

        Names requiredExtensions;
        Names requiredLayers;

        appendRequiredExtensions(requiredExtensions);
        appendRequiredLayers(requiredLayers);

        VkInstanceCreateFlags flags = enableCompatibility(requiredExtensions, availableExtensions);
        bool extensionRequirementsMet = testRequirements(requiredExtensions, availableExtensions);
        bool layerRequirementsMet = testRequirements(requiredLayers, availableLayers);

        meta::assert(extensionRequirementsMet, "one or more required Vulkan instance extensions are unavailable");
        meta::assert(layerRequirementsMet, "one or more required Vulkan instance layers are unavailable");

        auto appVersionMajor = applicationInfo_.version.major;
        auto appVersionMinor = applicationInfo_.version.minor;
        auto appVersionPatch = applicationInfo_.version.patch;

        auto engineVersionMajor = framework::VersionMajor;
        auto engineVersionMinor = framework::VersionMinor;
        auto engineVersionPatch = framework::VersionPatch;

        std::uint32_t appVersion = VK_MAKE_VERSION(
            appVersionMajor,
            appVersionMinor,
            appVersionPatch);

        std::uint32_t engineVersion = VK_MAKE_VERSION(
            engineVersionMajor,
            engineVersionMinor,
            engineVersionPatch);

        VkApplicationInfo applicationInfo = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = info.name.data(),
            .applicationVersion = appVersion,
            .pEngineName = framework::Name.data(),
            .engineVersion = engineVersion,
            .apiVersion = VK_API_VERSION_1_3,
        };

        VkInstanceCreateInfo createInfo = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
            .pApplicationInfo = &applicationInfo,
            .enabledLayerCount = static_cast<std::uint32_t>(requiredLayers.size()),
            .ppEnabledLayerNames = requiredLayers.data(),
            .enabledExtensionCount = static_cast<std::uint32_t>(requiredExtensions.size()),
            .ppEnabledExtensionNames = requiredExtensions.data(),
        };

        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance_);

        meta::assert(result == VK_SUCCESS, "Vulkan instance creation failed: {}", core::Vulkan_errorString(result));
        meta::logDebug(validation(), "Vulkan instance initialised");
    }

    inline void Instance::destroy() {
        if (instance_ == nullptr) {
            return;
        }

        vkDestroyInstance(instance_, nullptr);

        meta::logDebug(validation(), "Vulkan instance destroyed");

        applicationInfo_ = {};
        instance_ = nullptr;
    }

    inline bool Instance::valid() const {
        return instance_ != nullptr;
    }

    inline const core::ApplicationInfo& Instance::applicationInfo() const {
        return applicationInfo_;
    }

    inline bool Instance::validation() const {
        return applicationInfo_.features.validation;
    }

    inline Instance::ExtensionProperties Instance::getAvailableExtensions() const {
        std::uint32_t extensionCount = 0;

        VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        meta::assert(result == VK_SUCCESS, "Vulkan instance extension property enumeration failed: {}", core::Vulkan_errorString(result));

        ExtensionProperties extensions(extensionCount);

        result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        meta::assert(result == VK_SUCCESS, "Vulkan instance extension property enumeration failed: {}", core::Vulkan_errorString(result));

        return extensions;
    }

    inline Instance::LayerProperties Instance::getAvailableLayers() const {
        std::uint32_t layerCount = 0;

        VkResult result = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        meta::assert(result == VK_SUCCESS, "Vulkan instance layer property enumeration failed: {}", core::Vulkan_errorString(result));

        LayerProperties layers(layerCount);

        result = vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

        meta::assert(result == VK_SUCCESS, "Vulkan instance layer property enumeration failed: {}", core::Vulkan_errorString(result));

        return layers;
    }

    inline void Instance::appendRequiredExtensions(Names& requirements) const {
        std::uint32_t windowExtensionCount = 0;

        const char** windowExtensions = glfwGetRequiredInstanceExtensions(&windowExtensionCount);

        meta::assert(windowExtensions != nullptr, "GLFW provided no requirements instance extensions");

        requirements.reserve(windowExtensionCount);

        for (std::size_t i = 0; i < windowExtensionCount; ++i) {
            requirements.emplace_back(windowExtensions[i]);
        }
    }

    inline VkInstanceCreateFlags Instance::enableCompatibility(Names& requirements, const ExtensionProperties& available) const {
        bool found = false;

        for (auto& extension : available) {
            if (std::string_view(extension.extensionName) == VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME) {
                requirements.emplace_back(extension.extensionName);

                found = true;

                break;
            }
        }

        return found ? VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR : 0;
    }

    inline void Instance::appendRequiredLayers(Names& requirements) const {
        if (validation()) {
            requirements.emplace_back("VK_LAYER_KHRONOS_validation");
        }
    }

    inline bool Instance::testRequirements(const Names& requirements, const ExtensionProperties& available) const {
        bool succeeded = true;

        meta::logDebug(validation(), "required Vulkan instance extensions: {}", requirements.size());

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
                meta::logError("Vulkan instance extension is required but unsupported: {}", requirement);

                succeeded = false;
            }
        }

        return succeeded;
    }

    inline bool Instance::testRequirements(const Names& requirements, const LayerProperties& available) const {
        bool succeeded = true;

        meta::logDebug(validation(), "required Vulkan instance layers: {}", requirements.size());

        for (auto& requirement : requirements) {
            bool found = false;

            for (auto& candidate : available) {
                if (std::string_view(requirement) != candidate.layerName) {
                    continue;
                }

                found = true;

                meta::logDebugListElement(validation(), 1, "{}", requirement);

                break;
            }

            if (!found) {
                meta::logError("Vulkan instance layer is required but unsupported: {}", requirement);
                succeeded = false;
            }
        }

        return succeeded;
    }

    inline std::uint32_t Instance::getDriverSupportedVersion() const {
        std::uint32_t supportedVersion = 0;

        VkResult result = vkEnumerateInstanceVersion(&supportedVersion);

        meta::assert(result == VK_SUCCESS, "Vulkan API version query failed: {}", core::Vulkan_errorString(result));

        std::uint32_t major = VK_API_VERSION_MAJOR(supportedVersion);
        std::uint32_t minor = VK_API_VERSION_MINOR(supportedVersion);
        std::uint32_t patch = VK_API_VERSION_PATCH(supportedVersion);

        meta::logDebug(validation(), "driver Vulkan version: {}.{}.{}", major, minor, patch);

        return supportedVersion;
    }
}
