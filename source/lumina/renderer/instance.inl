#pragma once

#include "glfw.hpp"
#include "instance.hpp"

#include <lumina/framework/information.hpp>
#include <lumina/meta/console.hpp>
#include <lumina/meta/exceptions.hpp>

namespace lumina::renderer {
    inline Instance::Instance(const ApplicationInfo& info)
        : debugging_(info.features.debugging) {
        auto& features = info.features;

        initialiseSystemAPI();

        std::uint32_t driverVersion = getDriverSupportedVersion();

        uint32_t driverVersionMajor = VK_API_VERSION_MAJOR(driverVersion);
        uint32_t driverVersionMinor = VK_API_VERSION_MINOR(driverVersion);
        uint32_t driverVariant = VK_API_VERSION_VARIANT(driverVersion);

        if (driverVariant > 0) {
            throw meta::Exception(
                "non-standard Vulkan variant detected ({}), cannot proceed",
                driverVariant);
        }

        bool notVulkan1 = driverVersionMajor < 1;
        bool notVulkan13 = driverVersionMajor == 1 && driverVersionMinor < 3;

        if (notVulkan1 || notVulkan13) {
            throw meta::Exception(
                "Vulkan 1.3 requirements, system supports {}.{}",
                driverVersionMajor,
                driverVersionMinor);
        }

        ExtensionProperties availableExtensions = getAvailableExtensions();
        LayerProperties availableLayers = getAvailableLayers();

        Names requiredExtensions;
        Names requiredLayers;

        appendRequiredExtensions(features, requiredExtensions);
        appendRequiredLayers(features, requiredLayers);

        VkInstanceCreateFlags flags = enableCompatibility(requiredExtensions, availableExtensions);

        if (!testRequirements(requiredExtensions, availableExtensions)) {
            throw meta::Exception("one or more required Vulkan instance extensions are unavailable");
        }

        if (!testRequirements(requiredLayers, availableLayers)) {
            throw meta::Exception("one or more required Vulkan instance layers are unavailable");
        }

        auto appVersionMajor = info.version.major;
        auto appVersionMinor = info.version.minor;
        auto appVersionPatch = info.version.patch;

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

        if (result != VK_SUCCESS) {
            throw meta::Exception(
                "Vulkan instance creation failed: {}",
                Vulkan_errorString(result));
        }

        meta::logDebug(debugging_, "Vulkan instance initialised");
    }

    inline Instance::~Instance() {
        if (instance_ != nullptr) {
            vkDestroyInstance(instance_, nullptr);
            glfwTerminate();

            instance_ = nullptr;

            meta::logDebug(debugging_, "Vulkan instance destroyed");
        }
    }

    inline void Instance::update() {
        glfwPollEvents();
    }

    inline void Instance::initialiseSystemAPI() const {
        glfwSetErrorCallback(GLFW_errorCallback);
        glfwInit();

        // stop GLFW from creating windows with an OpenGL context
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }

    inline Instance::ExtensionProperties Instance::getAvailableExtensions() const {
        std::uint32_t extensionCount = 0;

        VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        if (result != VK_SUCCESS) {
            throw meta::Exception(
                "Vulkan instance extension property enumeration failed: {}",
                Vulkan_errorString(result));
        }

        ExtensionProperties extensions(extensionCount);

        result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        if (result != VK_SUCCESS) {
            throw meta::Exception(
                "Vulkan instance extension property enumeration failed: {}",
                Vulkan_errorString(result));
        }

        return extensions;
    }

    inline Instance::LayerProperties Instance::getAvailableLayers() const {
        std::uint32_t layerCount = 0;

        VkResult result = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        if (result != VK_SUCCESS) {
            throw meta::Exception(
                "Vulkan instance layer property enumeration failed: {}",
                Vulkan_errorString(result));
        }

        LayerProperties layers(layerCount);

        result = vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

        if (result != VK_SUCCESS) {
            throw meta::Exception(
                "Vulkan instance layer property enumeration failed: {}",
                Vulkan_errorString(result));
        }

        return layers;
    }

    inline void Instance::appendRequiredExtensions(const Features&, Names& requirements) const {
        std::uint32_t windowExtensionCount = 0;

        const char** windowExtensions = glfwGetRequiredInstanceExtensions(&windowExtensionCount);

        if (windowExtensions == nullptr) {
            throw meta::Exception("GLFW provided no requirements instance extensions");
        }

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

    inline void Instance::appendRequiredLayers(const Features& features, Names& requirements) const {
        if (features.validation) {
            requirements.emplace_back("VK_LAYER_KHRONOS_validation");
        }
    }

    inline bool Instance::testRequirements(const Names& requirements, const ExtensionProperties& available) const {
        bool succeeded = true;

        meta::logDebug(
            debugging_,
            "required Vulkan instance extensions: {}",
            requirements.size());

        for (auto& requirement : requirements) {
            bool found = false;

            for (auto& candidate : available) {
                if (std::string_view(requirement) == candidate.extensionName) {
                    found = true;

                    meta::logDebugListElement(
                        debugging_,
                        1,
                        "{}",
                        requirement);

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

        meta::logDebug(
            debugging_,
            "required Vulkan instance layers: {}",
            requirements.size());

        for (auto& requirement : requirements) {
            bool found = false;

            for (auto& candidate : available) {
                if (std::string_view(requirement) == candidate.layerName) {
                    found = true;

                    meta::logDebugListElement(
                        debugging_,
                        1,
                        "{}",
                        requirement);

                    break;
                }
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

        if (result != VK_SUCCESS) {
            throw meta::Exception(
                "Vulkan API version query failed: {}",
                Vulkan_errorString(result));
        }

        meta::logDebug(
            debugging_,
            "driver Vulkan version: {}.{}.{}",
            VK_API_VERSION_MAJOR(supportedVersion),
            VK_API_VERSION_MINOR(supportedVersion),
            VK_API_VERSION_PATCH(supportedVersion));

        return supportedVersion;
    }
}