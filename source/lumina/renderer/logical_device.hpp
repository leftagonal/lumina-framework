#pragma once

#include "vulkan.hpp"

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

    class Instance;
    class WindowManager;

    class LogicalDevice {
    public:
        LogicalDevice() = default;
        LogicalDevice(WindowManager& windowManager, const DeviceRequirements& requirements);
        ~LogicalDevice();

        LogicalDevice(const LogicalDevice&) = delete;
        LogicalDevice(LogicalDevice&&) noexcept;

        LogicalDevice& operator=(const LogicalDevice&) = delete;
        LogicalDevice& operator=(LogicalDevice&&) noexcept;

        [[nodiscard]] Instance& instance();
        [[nodiscard]] const Instance& instance() const;

        [[nodiscard]] WindowManager& windowManager();
        [[nodiscard]] const WindowManager& windowManager() const;

        void destroy();

        [[nodiscard]] bool valid() const;
        explicit operator bool() const;

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

        Instance* instance_ = nullptr;
        WindowManager* windowManager_ = nullptr;
        VkPhysicalDevice physicalDevice_ = nullptr;
        VkDevice device_ = nullptr;
        VkQueue presentQueue_ = nullptr;
        VkQueue graphicsQueue_ = nullptr;
        VkQueue computeQueue_ = nullptr;
        VkQueue transferQueue_ = nullptr;

        [[nodiscard]] bool validation() const;

        [[nodiscard]] QueueCreateInfos makeQueueCreateInfos(const QueueFamilySelections& queueSelections) const;
        [[nodiscard]] PhysicalDevices getAvailablePhysicalDevices() const;
        [[nodiscard]] QueueFamilies getAvailableQueueFamilies() const;
        [[nodiscard]] QueueFamilySelections pickSuitableQueueFamilies(const QueueFamilies& families) const;
        [[nodiscard]] ExtensionProperties getAvailableExtensions() const;
        [[nodiscard]] bool testRequirements(const Names& requirements, const ExtensionProperties& available) const;

        void pickMostSuitablePhysicalDevice(const PhysicalDevices& physicalDevices, const DeviceRequirements& deviceRequirements);
        void tryEnableCompatibility(Names& required, const ExtensionProperties& available) const;
    };
}
