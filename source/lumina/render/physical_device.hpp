#pragma once

#include "queue.hpp"

#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace lumina::render {
    enum class PhysicalDeviceType {
        Other,
        Integrated,
        Dedicated,
        Virtual,
        Software,
    };

    struct PhysicalDeviceLimits {
        std::uint32_t maximumImageDimension1D;
        std::uint32_t maximumImageDimension2D;
        std::uint32_t maximumImageDimension3D;
        std::uint32_t maximumUniformBufferRange;
        std::uint32_t maximumStorageBufferRange;
        std::uint32_t maximumPushConstantsSize;
        std::uint32_t maximumMemoryAllocationCount;
        std::uint32_t maximumBoundDescriptorSets;
        std::uint32_t maximumComputeWorkGroupInvocations;
        std::uint32_t maximumViewports;

        float maximumSamplerAnisotropy;
        float maximumSamplerLODBias;
    };

    struct PhysicalDeviceProperties {
        std::string_view name;
        std::uint32_t vendorID;
        std::uint32_t deviceID;
        std::uint32_t driverVersion;
        std::uint32_t apiVersion;
        PhysicalDeviceType deviceType;
        PhysicalDeviceLimits limits;
    };

    class PhysicalDevice {
    public:
        PhysicalDevice(VkPhysicalDevice physicalDevice)
            : physicalDevice_(physicalDevice) {
            vkGetPhysicalDeviceProperties(physicalDevice_, &properties_);

            std::uint32_t familyCount = 0;

            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &familyCount, nullptr);

            std::vector<VkQueueFamilyProperties> families(familyCount);
            queueFamilies_.reserve(familyCount);

            vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &familyCount, families.data());

            for (std::uint32_t i = 0; i < familyCount; ++i) {
                queueFamilies_.emplace_back(QueueFamily{
                    .definition = QueueFamilyDefinition{
                        .index = i,
                        .members = families[i].queueCount,
                    },
                    .features = QueueFamilyFeatures{
                        .graphics = (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 1,
                        .transfer = (families[i].queueFlags & VK_QUEUE_TRANSFER_BIT) == 1,
                        .compute = (families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 1,
                    },
                });
            }
        }

        ~PhysicalDevice() {
            physicalDevice_ = nullptr;
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
                .limits = PhysicalDeviceLimits{
                    .maximumImageDimension1D = limits.maxImageDimension1D,
                    .maximumImageDimension2D = limits.maxImageDimension2D,
                    .maximumImageDimension3D = limits.maxImageDimension3D,
                    .maximumUniformBufferRange = limits.maxUniformBufferRange,
                    .maximumStorageBufferRange = limits.maxStorageBufferRange,
                    .maximumPushConstantsSize = limits.maxPushConstantsSize,
                    .maximumMemoryAllocationCount = limits.maxMemoryAllocationCount,
                    .maximumBoundDescriptorSets = limits.maxBoundDescriptorSets,
                    .maximumComputeWorkGroupInvocations = limits.maxComputeWorkGroupInvocations,
                    .maximumViewports = limits.maxViewports,
                    .maximumSamplerAnisotropy = limits.maxSamplerAnisotropy,
                    .maximumSamplerLODBias = limits.maxSamplerLodBias,
                },
            };
        }

        [[nodiscard]] VkPhysicalDevice handle() const {
            return physicalDevice_;
        }

        void acquire() {
            ++acquisitions_;
        }

        void release() {
            if (acquisitions_ == 0) {
                return;
            }

            --acquisitions_;
        }

        [[nodiscard]] std::size_t acquisitions() const {
            return acquisitions_;
        }

        [[nodiscard]] std::span<const QueueFamily> queueFamilies() const {
            return queueFamilies_;
        }

    private:
        VkPhysicalDevice physicalDevice_;
        VkPhysicalDeviceProperties properties_ = {};

        std::vector<QueueFamily> queueFamilies_;

        std::size_t acquisitions_ = 0;

        [[nodiscard]] static PhysicalDeviceType mapDeviceType(VkPhysicalDeviceType type) {
            switch (type) {
                case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU: {
                    return PhysicalDeviceType::Integrated;
                }
                case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU: {
                    return PhysicalDeviceType::Dedicated;
                }
                case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU: {
                    return PhysicalDeviceType::Virtual;
                }
                case VK_PHYSICAL_DEVICE_TYPE_CPU: {
                    return PhysicalDeviceType::Software;
                }
                default: {
                    return PhysicalDeviceType::Other;
                }
            }
        }
    };
}