#pragma once

#include "../meta/resource.hpp"
#include "queue.hpp"

#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace lumina::render {
    /**
     * @brief The type of physical device.
     *
     */
    enum class PhysicalDeviceType {
        /// @brief Some other unrecognised type.
        Other,

        /// @brief Integrated graphics. (On-CPU)
        Integrated,

        /// @brief A dedicated graphics device.
        Dedicated,

        /// @brief A virtual display adapter. (Virtual machines)
        Virtual,

        /// @brief A CPU-driven display adapter.
        Software,
    };

    /**
     * @brief Limits imposed by the physical device.
     *
     */
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

    /**
     * @brief Information about the physical device.
     *
     */
    struct PhysicalDeviceProperties {
        std::string_view name;
        std::uint32_t vendorID;
        std::uint32_t deviceID;
        std::uint32_t driverVersion;
        std::uint32_t apiVersion;
        PhysicalDeviceType deviceType;
        PhysicalDeviceLimits limits;
    };

    /**
     * @brief A representation of a GPU.
     *
     */
    class PhysicalDevice : public Resource<VkPhysicalDevice> {
    public:
        /**
         * @brief Construct a new physical device.
         *
         * @param physicalDevice The handle for the physical device.
         *
         * Physical devices do not own their handle, the instance does.
         */
        PhysicalDevice(VkPhysicalDevice physicalDevice)
            : Resource(physicalDevice) {
            vkGetPhysicalDeviceProperties(resource(), &properties_);

            std::uint32_t familyCount = 0;

            vkGetPhysicalDeviceQueueFamilyProperties(resource(), &familyCount, nullptr);

            std::vector<VkQueueFamilyProperties> families(familyCount);
            queueFamilies_.reserve(familyCount);

            vkGetPhysicalDeviceQueueFamilyProperties(resource(), &familyCount, families.data());

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

        /**
         * @brief Release the physical device.
         *
         */
        ~PhysicalDevice() {
            if (!*this) {
                return;
            }

            release();

            properties_ = {};

            queueFamilies_.clear();
        }

        PhysicalDevice(PhysicalDevice&&) noexcept = default;

        PhysicalDevice& operator=(PhysicalDevice&&) noexcept = default;

        /**
         * @brief Get the properties of this physical device.
         *
         * @return PhysicalDeviceProperties The properties of this physical device.
         */
        [[nodiscard]] PhysicalDeviceProperties properties() const noexcept {
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

        /**
         * @brief Mark the physical device as in-use.
         *
         * This only exists for the logical device.
         */
        void acquire() {
            ++acquisitions_;
        }

        /**
         * @brief Releases a previous acquisition of the device.
         *
         * This only exists for the logical device.
         */
        void release() {
            if (acquisitions_ == 0) {
                return;
            }

            --acquisitions_;
        }

        /**
         * @brief Provides how many systems are using or have acquired this physical device.
         *
         */
        [[nodiscard]] std::size_t acquisitions() const {
            return acquisitions_;
        }

        /**
         * @brief Provides the queue families exposed by this device.
         *
         * @return std::span<const QueueFamily> The list of queue families.
         */
        [[nodiscard]] std::span<const QueueFamily> queueFamilies() const {
            return queueFamilies_;
        }

    private:
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