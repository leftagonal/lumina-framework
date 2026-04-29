#pragma once

#include <cstdint>
#include <cstdio>
#include <lumina/core/application.hpp>
#include <lumina/render/physical_device.hpp>
#include <string_view>
#include <vector>
#include <vulkan/vulkan.h>

namespace lumina::render {
    struct InstanceFeatures {
        bool platformIntegration;
        bool validation;
        bool debugging;
    };

    struct InstanceInfo {
        Application& application;
        InstanceFeatures features;
    };

    enum class PhysicalDeviceRequest {
        MostCapable,
        LeastCapable,
        NextAvailable,
    };

    class Instance {
    public:
        Instance(const InstanceInfo& info) {
            Extensions availableExtensions = getSupportedExtensions();
            Names platformExtensions = getPlatformExtensions();
            Names requiredExtensions = getRequiredExtensions(info, platformExtensions);

            if (!supportsRequestedExtensions(availableExtensions, requiredExtensions)) {
                std::printf("error: not all required instance extensions are supported\n");
                std::exit(1);
            }

            Layers availableLayers = getSupportedLayers();
            Names requiredLayers = getRequiredLayers(info, availableLayers);

            std::uint32_t applicationVersion = VK_MAKE_VERSION(
                info.application.version().major,
                info.application.version().minor,
                info.application.version().patch);

            std::uint32_t apiVersion = getApiVersion();

            std::uint32_t engineVersion = VK_MAKE_VERSION(
                0,
                1,
                0);

            VkApplicationInfo applicationInfo = {
                .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pNext = nullptr,
                .pApplicationName = info.application.name().data(),
                .applicationVersion = applicationVersion,
                .pEngineName = nullptr,
                .engineVersion = engineVersion,
                .apiVersion = apiVersion,
            };

            VkInstanceCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pNext = nullptr,
                .flags = portability_ ? VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR : 0u,
                .pApplicationInfo = &applicationInfo,
                .enabledLayerCount = static_cast<std::uint32_t>(requiredLayers.size()),
                .ppEnabledLayerNames = requiredLayers.data(),
                .enabledExtensionCount = static_cast<std::uint32_t>(requiredExtensions.size()),
                .ppEnabledExtensionNames = requiredExtensions.data(),
            };

            VkResult result = vkCreateInstance(&createInfo, nullptr, &instance_);

            if (result != VK_SUCCESS) {
                std::printf("error: failed to create instance\n");
                std::exit(1);
            }

            queryPhysicalDevices();
        }

        ~Instance() {
            if (instance_) {
                vkDestroyInstance(instance_, nullptr);
                instance_ = nullptr;
            }
        }

        [[nodiscard]] PhysicalDevice acquirePhysicalDevice(PhysicalDeviceRequest request) {
            if (physicalDevices_.empty()) {
                std::printf("error: no physical devices detected\n");
                std::exit(1);
            }

            std::size_t index = 0;

            switch (request) {
                case PhysicalDeviceRequest::MostCapable: {
                    index = 0;
                    break;
                }
                case PhysicalDeviceRequest::LeastCapable: {
                    index = physicalDevices_.size() - 1;
                    break;
                }
                case PhysicalDeviceRequest::NextAvailable: {
                    for (std::size_t i = 0; i < physicalDevices_.size(); ++i) {
                        if (deviceReferences_[index] > deviceReferences_[i]) {
                            index = i;
                        }
                    }
                }
            }

            return PhysicalDevice{
                physicalDevices_[index],
                &deviceReferences_[index],
            };
        }

        [[nodiscard]] VkInstance handle() const {
            return instance_;
        }

    private:
        using Names = std::vector<const char*>;
        using Extensions = std::vector<VkExtensionProperties>;
        using Layers = std::vector<VkLayerProperties>;
        using PhysicalDevices = std::vector<VkPhysicalDevice>;
        using DeviceReferences = std::vector<std::uint32_t>;

        PhysicalDevices physicalDevices_;
        DeviceReferences deviceReferences_;
        VkInstance instance_ = nullptr;
        bool portability_ = false;

        [[nodiscard]] Extensions getSupportedExtensions() {
            std::uint32_t extensionCount = 0;

            VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

            if (result != VK_SUCCESS) {
                std::printf("error: failed to enumerate supported instance extensions\n");
                std::exit(1);
            }

            Extensions extensions(extensionCount);

            result = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to enumerate supported instance extensions\n");
                std::exit(1);
            }

            return extensions;
        }

        [[nodiscard]] Layers getSupportedLayers() {
            std::uint32_t layerCount = 0;

            VkResult result = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            if (result != VK_SUCCESS) {
                std::printf("error: failed to enumerate supported instance layers\n");
                std::exit(1);
            }

            Layers layers(layerCount);

            result = vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to enumerate supported instance layers\n");
                std::exit(1);
            }

            return layers;
        }

        [[nodiscard]] Names getPlatformExtensions() {
            std::uint32_t platformExtensionCount = 0;

            const char** extensions = glfwGetRequiredInstanceExtensions(&platformExtensionCount);

            if (extensions == nullptr) {
                std::printf("error: failed to query platform extensions\n");
                std::exit(1);
            }

            return Names{
                extensions,
                extensions + platformExtensionCount,
            };
        }

        [[nodiscard]] Names getRequiredExtensions(const InstanceInfo& info, const Names& candidates) {
            Names requiredExtensions;

            constexpr const char* PortabilityEnumeration = "VK_KHR_portability_enumeration";

            for (auto extension : candidates) {
                bool isPortabilityEnumeration = std::string_view(extension) == PortabilityEnumeration;

                portability_ = portability_ || isPortabilityEnumeration;

                // any required extensions should be included if requested
                // this excludes portability extensions since they are always neededa
                if (info.features.platformIntegration && !isPortabilityEnumeration) {
                    requiredExtensions.emplace_back(extension);
                }
            }

            if (portability_) {
                requiredExtensions.emplace_back(PortabilityEnumeration);
            }

            return requiredExtensions;
        }

        [[nodiscard]] Names getRequiredLayers(const InstanceInfo& info, const Layers& supported) {
            Names requiredLayers;

            bool foundValidation = false;

            constexpr const char* Validation = "VK_LAYER_KHRONOS_validation";

            for (auto candidate : supported) {
                bool isValidation = std::string_view(candidate.layerName) == Validation;

                foundValidation = foundValidation || isValidation;
            }

            if (info.features.validation) {
                if (foundValidation) {
                    requiredLayers.emplace_back(Validation);
                } else {
                    std::printf("error: validation layer unavailable\n");
                    std::exit(1);
                }
            }

            return requiredLayers;
        }

        [[nodiscard]] bool supportsRequestedExtensions(const Extensions& extensions, const Names& requirements) {
            for (auto& requirement : requirements) {
                bool found = false;

                for (auto& available : extensions) {
                    bool match = std::string_view(requirement) == available.extensionName;

                    found = found || match;
                }

                if (!found) {
                    return false;
                }
            }

            return true;
        }

        [[nodiscard]] std::uint32_t getApiVersion() {
            std::uint32_t version = 0;

            constexpr std::uint32_t TargetVersion = VK_API_VERSION_1_3;

            VkResult result = vkEnumerateInstanceVersion(&version);

            if (result != VK_SUCCESS) {
                std::printf("error: failed to query API version\n");
                std::exit(1);
            }

            return std::min(version, TargetVersion);
        }

        [[nodiscard]] float ratePhysicalDevice(VkPhysicalDevice physicalDevice) {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(physicalDevice, &properties);

            float rating = 1.0f;

            switch (properties.deviceType) {
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: {
                    rating *= 1000.0f;
                    break;
                }
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: {
                    rating *= 100.0f;
                    break;
                }
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: {
                    rating *= 50.0f;
                    break;
                }
                case VK_PHYSICAL_DEVICE_TYPE_CPU: {
                    rating *= 10.0f;
                    break;
                }
                default: {
                    break;
                }
            }

            const auto& limits = properties.limits;

            rating += static_cast<float>(limits.maxImageDimension2D) / 1000.0f;
            rating += static_cast<float>(limits.maxPushConstantsSize) / 16.0f;
            rating += static_cast<float>(limits.maxComputeWorkGroupInvocations) / 256.0f;
            rating += static_cast<float>(limits.framebufferColorSampleCounts) * 0.5f;

            return rating;
        }

        void queryPhysicalDevices() {
            physicalDevices_.clear();

            std::uint32_t physicalDeviceCount = 0;

            VkResult result = vkEnumeratePhysicalDevices(instance_, &physicalDeviceCount, nullptr);

            if (result != VK_SUCCESS) {
                std::printf("error: failed to query physical devices\n");
                std::exit(1);
            }

            physicalDevices_.resize(physicalDeviceCount, nullptr);
            deviceReferences_.resize(physicalDeviceCount, 0);

            result = vkEnumeratePhysicalDevices(instance_, &physicalDeviceCount, physicalDevices_.data());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to query physical devices\n");
                std::exit(1);
            }

            // sort physical devices based on their capabilities
            // sorting should be from "most capable" to "least capable"

            auto sorter = [this](const auto& left, const auto& right) {
                return ratePhysicalDevice(left) > ratePhysicalDevice(right);
            };

            std::sort(physicalDevices_.begin(), physicalDevices_.end(), sorter);
        }
    };
}