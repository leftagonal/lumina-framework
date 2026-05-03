#pragma once

#include "../meta/resource.hpp"
#include "../resources/queue.hpp"
#include "instance.hpp"

#include <span>

namespace lumina::render {
    /**
     * @brief Definitions for the queues that a logical device should be created with.
     *
     * Queue family definitions should be selected and constructed with information provided
     * by the selected physical device, as well as the surface if presentation is required.
     */
    struct DeviceQueueDefinitions {
        std::vector<QueueFamilyDefinition> definitions;
    };

    /**
     * @brief The features a logical device should support.
     *
     * Any unsupported features being required by this will currently cause a program failure.
     */
    struct DeviceFeatures {
        bool swapchains;
    };

    /**
     * @brief A logical device for interacting with the GPU.
     *
     */
    class Device : public Resource<VkDevice> {
    public:
        Device() = default;

        /**
         * @brief Construct a new logical device.
         *
         * @param instance The instance to construct the device for.
         * @param physicalDevice The physical device (GPU) to interface with.
         * @param queues The queue requirements of this device.
         * @param features The expected features to be supported by this device.
         */
        Device(Instance& instance, PhysicalDevice& physicalDevice, const DeviceQueueDefinitions& queues, const DeviceFeatures& features)
            : instance_(&instance), physicalDevice_(&physicalDevice), queueFamilies_(queues.definitions) {
            Extensions supportedExtensions = getSupportedExtensions();
            Names selectedExtensions = getSelectedExtensions(supportedExtensions, features);
            QueueCreateInfos queueCreateInfos = makeQueueCreateInfos();

            createDevice(selectedExtensions, queueCreateInfos);
            populateQueues();
        }

        /**
         * @brief Destroy the logical device.
         *
         */
        ~Device() {
            destroy();
        }

        Device(Device&&) noexcept = default;

        Device& operator=(Device&&) noexcept = default;

        /**
         * @brief Construct the device.
         *
         * @param instance The instance to construct the device for.
         * @param physicalDevice The physical device (GPU) to interface with.
         * @param queues The queue requirements of this device.
         * @param features The expected features to be supported by this device.
         */
        void create(Instance& instance, PhysicalDevice& physicalDevice, const DeviceQueueDefinitions& queueDefinitions, const DeviceFeatures& features) {
            if (*this) {
                return;
            }

            instance_ = &instance;
            physicalDevice_ = &physicalDevice;
            queueFamilies_ = std::move(queueDefinitions.definitions);

            Extensions supportedExtensions = getSupportedExtensions();
            Names selectedExtensions = getSelectedExtensions(supportedExtensions, features);
            QueueCreateInfos queueCreateInfos = makeQueueCreateInfos();

            createDevice(selectedExtensions, queueCreateInfos);
            populateQueues();
        }

        /**
         * @brief Destroys the logical device.
         *
         */
        void destroy() {
            if (!*this) {
                return;
            }

            vkDestroyDevice(resource(), nullptr);
            physicalDevice_->release();

            instance_ = nullptr;
            physicalDevice_ = nullptr;

            queues_.clear();
            queueFamilies_.clear();

            invalidate();
        }

        /**
         * @brief The physical device this logical device is associated with.
         *
         * @return PhysicalDevice& The physical device.
         */
        [[nodiscard]] PhysicalDevice& physicalDevice() {
            return *physicalDevice_;
        }

        /**
         * @brief The physical device this logical device is associated with.
         *
         * @return const PhysicalDevice& The physical device.
         */
        [[nodiscard]] const PhysicalDevice& physicalDevice() const {
            return *physicalDevice_;
        }

        /**
         * @brief The instance this logical device is associated with.
         *
         * @return Instance& The instance.
         */
        [[nodiscard]] Instance& instance() {
            return *instance_;
        }

        /**
         * @brief The instance this logical device is associated with.
         *
         * @return const Instance& The instance.
         */
        [[nodiscard]] const Instance& instance() const {
            return *instance_;
        }

        /**
         * @brief Get all available queues that are a member of the provided queue family.
         *
         * @param familyIndex The queue family index to query.
         * @return std::span<Queue> The list of queues.
         *
         * Queues are owned by the logical device.
         */
        [[nodiscard]] std::span<Queue> getQueueFamily(std::uint32_t familyIndex) {
            std::int64_t offset = 0;

            for (std::size_t i = 0; i < familyIndex; ++i) {
                auto& family = queueFamilies_[i];
                offset += family.members;
            }

            auto& family = queueFamilies_[familyIndex];

            return {
                queues_.begin() + offset,
                family.members,
            };
        }

        /**
         * @brief Get all available queues that are a member of the provided queue family.
         *
         * @param familyIndex The queue family index to query.
         * @return std::span<const Queue> The list of queues.
         *
         * Queues are owned by the logical device.
         */
        [[nodiscard]] std::span<const Queue> getQueueFamily(std::uint32_t familyIndex) const {
            std::int64_t offset = 0;

            for (std::size_t i = 0; i < familyIndex; ++i) {
                auto& family = queueFamilies_[i];
                offset += family.members;
            }

            auto& family = queueFamilies_[familyIndex];

            return {
                queues_.begin() + offset,
                family.members,
            };
        }

    private:
        using Extensions = std::vector<VkExtensionProperties>;
        using Names = std::vector<const char*>;

        struct QueueCreateInfos {
            std::vector<VkDeviceQueueCreateInfo> createInfos;
            std::vector<std::vector<float>> priorities;
        };

        Instance* instance_;
        PhysicalDevice* physicalDevice_;

        std::vector<Queue> queues_;
        std::vector<QueueFamilyDefinition> queueFamilies_;

        [[nodiscard]] Extensions getSupportedExtensions() const {
            std::uint32_t extensionCount = 0;

            VkResult result = vkEnumerateDeviceExtensionProperties(*physicalDevice_, nullptr, &extensionCount, nullptr);

            if (result != VK_SUCCESS) {
                std::printf("error: failed to enumerate device extensions\n");
                std::exit(1);
            }

            std::vector<VkExtensionProperties> extensions(extensionCount);

            result = vkEnumerateDeviceExtensionProperties(*physicalDevice_, nullptr, &extensionCount, extensions.data());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to enumerate device extensions\n");
                std::exit(1);
            }

            return extensions;
        }

        [[nodiscard]] Names getSelectedExtensions(const Extensions& extensions, const DeviceFeatures& features) const {
            Names selectedExtensions;

            bool swapchainSupported = false;
            bool needsPortability = false;

            constexpr const char* Swapchain = "VK_KHR_swapchain";
            constexpr const char* PortabilitySubset = "VK_KHR_portability_subset";

            for (auto& extension : extensions) {
                bool isPortabilitySubset = std::string_view(extension.extensionName) == PortabilitySubset;

                needsPortability = needsPortability || isPortabilitySubset;

                if (std::string_view(extension.extensionName) == Swapchain) {
                    swapchainSupported = true;
                    selectedExtensions.emplace_back(extension.extensionName);
                }
            }

            if (features.swapchains && !swapchainSupported) {
                std::printf("error: presentation is required but is unsupported by the selected physical device\n");
                std::exit(1);
            }

            if (needsPortability) {
                selectedExtensions.emplace_back(PortabilitySubset);
            }

            return selectedExtensions;
        }

        [[nodiscard]] QueueCreateInfos makeQueueCreateInfos() const {
            QueueCreateInfos createInfos;

            createInfos.createInfos.reserve(queueFamilies_.size());

            for (auto& definition : queueFamilies_) {
                createInfos.priorities.emplace_back(definition.members, 1.0);
                createInfos.createInfos.emplace_back(VkDeviceQueueCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .queueFamilyIndex = definition.index,
                    .queueCount = definition.members,
                    .pQueuePriorities = createInfos.priorities.back().data(),
                });
            }

            return createInfos;
        }

        void populateQueues() {
            auto queueFamilies = physicalDevice_->queueFamilies();

            for (auto& family : queueFamilies_) {
                queues_.reserve(family.members);

                for (std::uint32_t i = 0; i < family.members; ++i) {
                    VkQueue queue;

                    vkGetDeviceQueue(resource(), family.index, i, &queue);

                    queues_.emplace_back(Queue{
                        queue,
                        QueueAssignment{
                            .familyIndex = family.index,
                            .queueIndex = i,
                        },
                        queueFamilies[family.index].features,
                    });
                }
            }
        }

        void createDevice(const Names& selectedExtensions, const QueueCreateInfos& queueCreateInfos) {
            VkPhysicalDeviceVulkan11Features vulkan11Features = {};
            VkPhysicalDeviceVulkan12Features vulkan12Features = {};
            VkPhysicalDeviceVulkan13Features vulkan13Features = {};

            vulkan11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
            vulkan11Features.pNext = &vulkan12Features;

            vulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
            vulkan12Features.pNext = &vulkan13Features;
            vulkan12Features.timelineSemaphore = true;

            vulkan13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
            vulkan13Features.pNext = nullptr;
            vulkan13Features.synchronization2 = true;
            vulkan13Features.dynamicRendering = true;

            VkDeviceCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                .pNext = &vulkan11Features,
                .flags = 0,
                .queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.createInfos.size()),
                .pQueueCreateInfos = queueCreateInfos.createInfos.data(),
                .enabledLayerCount = 0,
                .ppEnabledLayerNames = nullptr,
                .enabledExtensionCount = static_cast<std::uint32_t>(selectedExtensions.size()),
                .ppEnabledExtensionNames = selectedExtensions.data(),
                .pEnabledFeatures = nullptr,
            };

            VkResult result = vkCreateDevice(*physicalDevice_, &createInfo, nullptr, &resource());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to create device\n");
                std::exit(1);
            }

            physicalDevice_->acquire();
        }
    };
}