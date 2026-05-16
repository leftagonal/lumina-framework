#pragma once

#include "queue.hpp"
#include "surface_manager.hpp"

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

    namespace accessors {
        class LogicalDeviceAccessor;
    }

    class LogicalDevice {
    public:
        LogicalDevice() = default;
        ~LogicalDevice();

        LogicalDevice(Instance& instance, SurfaceManager& surfaceManager, const DeviceRequirements& requirements);

        LogicalDevice(const LogicalDevice&) = delete;
        LogicalDevice(LogicalDevice&&) noexcept;

        LogicalDevice& operator=(const LogicalDevice&) = delete;
        LogicalDevice& operator=(LogicalDevice&&) noexcept;

        explicit operator bool() const;

        void create(Instance& instance, SurfaceManager& surfaceManager, const DeviceRequirements& requirements);
        void destroy();

        [[nodiscard]] Instance& instance();
        [[nodiscard]] const Instance& instance() const;

        [[nodiscard]] bool valid() const;

    private:
        struct QueueEntry {
            QueueFamilySelection selection;
            QueueFamilyDefinition definition;
        };

        struct QueueSelection {
            QueueEntry entry;
            QueueIntent intent;
            bool fallback;
        };

        using QueueSelections = std::vector<QueueSelection>;

        struct QueueCreateInfos {
            std::vector<VkDeviceQueueCreateInfo> createInfos;
            std::vector<std::vector<float>> queuePriorities;
        };

        using PhysicalDevices = std::vector<VkPhysicalDevice>;
        using QueueFamilies = std::vector<VkQueueFamilyProperties>;
        using ExtensionProperties = std::vector<VkExtensionProperties>;
        using Names = std::vector<const char*>;

        Instance* instance_ = nullptr;

        VkPhysicalDevice physicalDevice_ = nullptr;
        VkDevice device_ = nullptr;

        std::vector<QueueEntry> presentQueues_;
        std::vector<QueueEntry> graphicsQueues_;
        std::vector<QueueEntry> computeQueues_;
        std::vector<QueueEntry> transferQueues_;

        [[nodiscard]] bool validation() const;

        [[nodiscard]] QueueCreateInfos makeQueueCreateInfos(const QueueSelections& queueSelections) const;
        [[nodiscard]] PhysicalDevices getAvailablePhysicalDevices() const;
        [[nodiscard]] QueueFamilies getAvailableQueueFamilies() const;
        [[nodiscard]] QueueSelections pickSuitableQueueFamilies(const QueueFamilies& families, SurfaceManager& surfaceManager) const;
        [[nodiscard]] ExtensionProperties getAvailableExtensions() const;
        [[nodiscard]] bool testRequirements(const Names& requirements, const ExtensionProperties& available) const;

        void pickMostSuitablePhysicalDevice(const PhysicalDevices& physicalDevices, const DeviceRequirements& deviceRequirements);
        void tryEnableCompatibility(Names& required, const ExtensionProperties& available) const;

        friend class accessors::LogicalDeviceAccessor;
    };

    namespace accessors {
        class LogicalDeviceAccessor {
        public:
            LogicalDeviceAccessor() = delete;

            [[nodiscard]] static VkDevice& device(LogicalDevice& logicalDevice) {
                return logicalDevice.device_;
            }

            [[nodiscard]] static const VkDevice& device(const LogicalDevice& logicalDevice) {
                return logicalDevice.device_;
            }

            [[nodiscard]] static VkPhysicalDevice& physicalDevice(LogicalDevice& logicalDevice) {
                return logicalDevice.physicalDevice_;
            }

            [[nodiscard]] static const VkPhysicalDevice& physicalDevice(const LogicalDevice& logicalDevice) {
                return logicalDevice.physicalDevice_;
            }

            [[nodiscard]] static LogicalDevice::QueueEntry queueEntry(const LogicalDevice& logicalDevice, QueueIntent intent, std::size_t index) {
                auto getQueueEntry = [&](const std::vector<LogicalDevice::QueueEntry>& options) {
                    return options[index];
                };

                switch (intent) {
                    case QueueIntent::Present: {
                        return getQueueEntry(logicalDevice.presentQueues_);
                    }
                    case QueueIntent::Graphics: {
                        return getQueueEntry(logicalDevice.graphicsQueues_);
                    }
                    case QueueIntent::Compute: {
                        return getQueueEntry(logicalDevice.computeQueues_);
                    }
                    case QueueIntent::Transfer: {
                        return getQueueEntry(logicalDevice.transferQueues_);
                    }
                }
            }

            [[nodiscard]] static VkQueue queue(const LogicalDevice& logicalDevice, QueueIntent intent, std::size_t index) {
                auto getQueue = [&](const std::vector<LogicalDevice::QueueEntry>& options) {
                    auto& entry = options[index];

                    VkQueue queue = nullptr;

                    vkGetDeviceQueue(logicalDevice.device_, entry.selection.familyIndex, entry.selection.queueIndex, &queue);

                    return queue;
                };

                switch (intent) {
                    case QueueIntent::Present: {
                        return getQueue(logicalDevice.presentQueues_);
                    }
                    case QueueIntent::Graphics: {
                        return getQueue(logicalDevice.graphicsQueues_);
                    }
                    case QueueIntent::Compute: {
                        return getQueue(logicalDevice.computeQueues_);
                    }
                    case QueueIntent::Transfer: {
                        return getQueue(logicalDevice.transferQueues_);
                    }
                }
            }
        };
    }
}
