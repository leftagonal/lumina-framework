#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace lumina::renderer {
    enum class DeviceType {
        Integrated,
        Discrete,
        Software,
        Virtual,
        Other,
    };

    struct DeviceRequirements {
    };

    class Presenter;
    class Instance;

    class Device {
    public:
        Device(Presenter& presenter, const DeviceRequirements& requirements);
        ~Device();

        [[nodiscard]] Instance& instance() {
            return *instance_;
        }

        [[nodiscard]] const Instance& instance() const {
            return *instance_;
        }

    private:
        struct QueueFamilySelection {
            std::uint32_t familyIndex = 0;
            std::uint32_t queueIndex = 0;
        };

        using QueueFamilySelections = std::array<QueueFamilySelection, 4>;

        struct QueueCreateInfos {
            std::vector<VkDeviceQueueCreateInfo> createInfos;
            std::vector<std::vector<float>> queuePriorities;
        };

        using PhysicalDevices = std::vector<VkPhysicalDevice>;
        using QueueFamilies = std::vector<VkQueueFamilyProperties>;
        using ExtensionProperties = std::vector<VkExtensionProperties>;
        using Names = std::vector<const char*>;

        VkPhysicalDevice physicalDevice_ = nullptr;
        VkDevice device_ = nullptr;
        bool debugging_;

        VkQueue presentQueue_ = nullptr;
        VkQueue graphicsQueue_ = nullptr;
        VkQueue computeQueue_ = nullptr;
        VkQueue transferQueue_ = nullptr;

        Instance* instance_ = nullptr;

        [[nodiscard]] QueueCreateInfos makeQueueCreateInfos(const QueueFamilySelections& queueSelections) const;
        [[nodiscard]] PhysicalDevices getAvailablePhysicalDevices() const;
        [[nodiscard]] QueueFamilies getAvailableQueueFamilies() const;
        [[nodiscard]] QueueFamilySelections pickSuitableQueueFamilies(const QueueFamilies& families, Presenter& presenter) const;
        [[nodiscard]] ExtensionProperties getAvailableExtensions() const;
        [[nodiscard]] bool testRequirements(const Names& requirements, const ExtensionProperties& available) const;

        void pickMostSuitablePhysicalDevice(const PhysicalDevices& physicalDevices, const DeviceRequirements& deviceRequirements);
        void tryEnableCompatibility(Names& required, const ExtensionProperties& available) const;
    };
}