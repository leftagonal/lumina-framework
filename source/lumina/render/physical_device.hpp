#pragma once

#include <cstdint>
#include <string_view>
#include <vulkan/vulkan.h>

namespace lumina::render {
    enum class DeviceType {
        Other,
        IntegratedGPU,
        DiscreteGPU,
        VirtualGPU,
        CPU,
    };

    struct DeviceLimits {
        std::uint32_t maxImageDimension1D;
        std::uint32_t maxImageDimension2D;
        std::uint32_t maxImageDimension3D;
        std::uint32_t maxUniformBufferRange;
        std::uint32_t maxStorageBufferRange;
        std::uint32_t maxPushConstantsSize;
        std::uint32_t maxMemoryAllocationCount;
        std::uint32_t maxBoundDescriptorSets;
        std::uint32_t maxComputeWorkGroupInvocations;
        std::uint32_t maxViewports;

        float maxSamplerAnisotropy;
        float maxSamplerLodBias;
    };

    struct PhysicalDeviceProperties {
        std::string_view name;
        std::uint32_t vendorID;
        std::uint32_t deviceID;
        std::uint32_t driverVersion;
        std::uint32_t apiVersion;
        DeviceType deviceType;
        DeviceLimits limits;
    };

    class PhysicalDevice {
    public:
        PhysicalDevice(VkPhysicalDevice physicalDevice, std::uint32_t* references)
            : physicalDevice_(physicalDevice), references_(references) {
            vkGetPhysicalDeviceProperties(physicalDevice_, &properties_);

            ++(*references_);
        }

        ~PhysicalDevice() {
            --(*references_);
        }

        PhysicalDevice(const PhysicalDevice&) = default;
        PhysicalDevice(PhysicalDevice&&) = default;

        PhysicalDevice& operator=(const PhysicalDevice&) = default;
        PhysicalDevice& operator=(PhysicalDevice&&) = default;

        [[nodiscard]] PhysicalDeviceProperties properties() const {
            const auto& limits = properties_.limits;

            return PhysicalDeviceProperties{
                .name = properties_.deviceName,
                .vendorID = properties_.vendorID,
                .deviceID = properties_.deviceID,
                .driverVersion = properties_.driverVersion,
                .apiVersion = properties_.apiVersion,
                .deviceType = mapDeviceType(properties_.deviceType),
                .limits = DeviceLimits{
                    .maxImageDimension1D = limits.maxImageDimension1D,
                    .maxImageDimension2D = limits.maxImageDimension2D,
                    .maxImageDimension3D = limits.maxImageDimension3D,
                    .maxUniformBufferRange = limits.maxUniformBufferRange,
                    .maxStorageBufferRange = limits.maxStorageBufferRange,
                    .maxPushConstantsSize = limits.maxPushConstantsSize,
                    .maxMemoryAllocationCount = limits.maxMemoryAllocationCount,
                    .maxBoundDescriptorSets = limits.maxBoundDescriptorSets,
                    .maxComputeWorkGroupInvocations = limits.maxComputeWorkGroupInvocations,
                    .maxViewports = limits.maxViewports,
                    .maxSamplerAnisotropy = limits.maxSamplerAnisotropy,
                    .maxSamplerLodBias = limits.maxSamplerLodBias,
                },
            };
        }

        [[nodiscard]] VkPhysicalDevice handle() const {
            return physicalDevice_;
        }

    private:
        VkPhysicalDevice physicalDevice_;
        VkPhysicalDeviceProperties properties_;
        std::uint32_t* references_;

        [[nodiscard]] static DeviceType mapDeviceType(VkPhysicalDeviceType type) {
            switch (type) {
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: {
                    return DeviceType::IntegratedGPU;
                }
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: {
                    return DeviceType::DiscreteGPU;
                }
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: {
                    return DeviceType::VirtualGPU;
                }
                case VK_PHYSICAL_DEVICE_TYPE_CPU: {
                    return DeviceType::CPU;
                }
                default: {
                    return DeviceType::Other;
                }
            }
        }
    };
}