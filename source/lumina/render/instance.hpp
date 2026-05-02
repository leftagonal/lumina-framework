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

    enum class PhysicalDeviceRequest {
        MostCapable,
        LeastCapable,
        NextBestAvailable,
        NextWorstAvailable,
    };

    class Instance {
    public:
        Instance(const Application& application, const InstanceFeatures& features) {
            Extensions availableExtensions = getSupportedExtensions();
            Names platformExtensions = getPlatformExtensions();
            Names requiredExtensions = getRequiredExtensions(features, platformExtensions);

            if (!supportsRequestedExtensions(availableExtensions, requiredExtensions)) {
                std::printf("error: not all required instance extensions are supported\n");
                std::exit(1);
            }

            Layers availableLayers = getSupportedLayers();
            Names requiredLayers = getRequiredLayers(features, availableLayers);

            std::uint32_t applicationVersion = VK_MAKE_VERSION(
                application.version().major,
                application.version().minor,
                application.version().patch);

            std::uint32_t apiVersion = getApiVersion();

            std::uint32_t engineVersion = VK_MAKE_VERSION(
                0,
                1,
                0);

            VkApplicationInfo applicationInfo = {
                .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pNext = nullptr,
                .pApplicationName = application.name().data(),
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

        [[nodiscard]] PhysicalDevice& acquirePhysicalDevice(PhysicalDeviceRequest request) {
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
                case PhysicalDeviceRequest::NextBestAvailable: {
                    // begin with the first device
                    index = 0;

                    // forward iteration starts at the most capable and moves to the least capable
                    for (std::size_t i = 1; i < physicalDevices_.size(); ++i) {
                        auto& current = physicalDevices_[index];
                        auto& candidate = physicalDevices_[i];

                        // progressively see if there are devices that have fewer acquisitions
                        // if one has fewer acquisitions, cache its index
                        if (candidate.acquisitions() < current.acquisitions()) {
                            index = i;
                        }
                    }

                    break;
                }
                case PhysicalDeviceRequest::NextWorstAvailable: {
                    // begin with the last device
                    index = physicalDevices_.size() - 1;

                    // reverse iteration starts at the least capable and moves to the most capable
                    for (std::size_t i = physicalDevices_.size() - 1; i-- > 0;) {
                        auto& current = physicalDevices_[index];
                        auto& candidate = physicalDevices_[i];

                        // progressively see if there are devices that have fewer acquisitions
                        // if one has fewer acquisitions, cache its index
                        if (candidate.acquisitions() < current.acquisitions()) {
                            index = i;
                        }
                    }

                    break;
                }
            }

            return physicalDevices_[index];
        }

        [[nodiscard]] const PhysicalDevice& acquirePhysicalDevice(PhysicalDeviceRequest request) const {
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
                case PhysicalDeviceRequest::NextBestAvailable: {
                    // begin with the first device
                    index = 0;

                    // forward iteration starts at the most capable and moves to the least capable
                    for (std::size_t i = 1; i < physicalDevices_.size(); ++i) {
                        auto& current = physicalDevices_[index];
                        auto& candidate = physicalDevices_[i];

                        // progressively see if there are devices that have fewer acquisitions
                        // if one has fewer acquisitions, cache its index
                        if (candidate.acquisitions() < current.acquisitions()) {
                            index = i;
                        }
                    }

                    break;
                }
                case PhysicalDeviceRequest::NextWorstAvailable: {
                    // begin with the last device
                    index = physicalDevices_.size() - 1;

                    // reverse iteration starts at the least capable and moves to the most capable
                    for (std::size_t i = physicalDevices_.size() - 1; i-- > 0;) {
                        auto& current = physicalDevices_[index];
                        auto& candidate = physicalDevices_[i];

                        // progressively see if there are devices that have fewer acquisitions
                        // if one has fewer acquisitions, cache its index
                        if (candidate.acquisitions() < current.acquisitions()) {
                            index = i;
                        }
                    }

                    break;
                }
            }

            return physicalDevices_[index];
        }

        [[nodiscard]] VkInstance handle() const {
            return instance_;
        }

    private:
        using Names = std::vector<const char*>;
        using Extensions = std::vector<VkExtensionProperties>;
        using Layers = std::vector<VkLayerProperties>;

        using PhysicalDevices = std::vector<PhysicalDevice>;

        PhysicalDevices physicalDevices_;
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

        [[nodiscard]] Names getRequiredExtensions(const InstanceFeatures& features, const Names& candidates) {
            Names requiredExtensions;

            constexpr const char* PortabilityEnumeration = "VK_KHR_portability_enumeration";

            for (auto extension : candidates) {
                bool isPortabilityEnumeration = std::string_view(extension) == PortabilityEnumeration;

                portability_ = portability_ || isPortabilityEnumeration;

                // any required extensions should be included if requested
                // this excludes portability extensions since they are always neededa
                if (features.platformIntegration && !isPortabilityEnumeration) {
                    requiredExtensions.emplace_back(extension);
                }
            }

            if (portability_) {
                requiredExtensions.emplace_back(PortabilityEnumeration);
            }

            return requiredExtensions;
        }

        [[nodiscard]] Names getRequiredLayers(const InstanceFeatures& features, const Layers& supported) {
            Names requiredLayers;

            bool foundValidation = false;

            constexpr const char* Validation = "VK_LAYER_KHRONOS_validation";

            for (auto candidate : supported) {
                bool isValidation = std::string_view(candidate.layerName) == Validation;

                foundValidation = foundValidation || isValidation;
            }

            if (features.validation) {
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

        [[nodiscard]] float ratePhysicalDevice(PhysicalDevice physicalDevice) {
            PhysicalDeviceProperties properties = physicalDevice.properties();

            float rating = 1.0f;

            switch (properties.deviceType) {
                case PhysicalDeviceType::Dedicated: {
                    rating *= 1000.0f;
                    break;
                }
                case PhysicalDeviceType::Integrated: {
                    rating *= 100.0f;
                    break;
                }
                case PhysicalDeviceType::Virtual: {
                    rating *= 50.0f;
                    break;
                }
                case PhysicalDeviceType::Software: {
                    rating *= 10.0f;
                    break;
                }
                default: {
                    break;
                }
            }

            const auto& limits = properties.limits;

            rating += static_cast<float>(limits.maximumImageDimension2D) / 1000.0f;
            rating += static_cast<float>(limits.maximumPushConstantsSize) / 16.0f;
            rating += static_cast<float>(limits.maximumComputeWorkGroupInvocations) / 256.0f;

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

            std::vector<VkPhysicalDevice> physicalDevices;

            physicalDevices.resize(physicalDeviceCount, nullptr);
            physicalDevices_.reserve(physicalDevices.size());

            result = vkEnumeratePhysicalDevices(instance_, &physicalDeviceCount, physicalDevices.data());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to query physical devices\n");
                std::exit(1);
            }

            for (auto& device : physicalDevices) {
                physicalDevices_.emplace_back(device);
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