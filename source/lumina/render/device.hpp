#pragma once

#include "instance.hpp"
#include "queue.hpp"

#include <span>

namespace lumina::render {
    struct DeviceQueues {
        std::vector<QueueFamilyDefinition> definitions;
    };

    struct DeviceFeatures {
        bool swapchains;
    };

    class Device {
    public:
        Device(Instance& instance, PhysicalDevice& physicalDevice, DeviceQueues& queues, DeviceFeatures& features)
            : instance_(&instance), physicalDevice_(&physicalDevice), queueFamilies_(queues.definitions) {
            Extensions extensions = getExtensions();
            Names selectedExtensions;
            bool swapchainSupported = false;

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            std::vector<std::vector<float>> queuePriorities;

            queueCreateInfos.reserve(queues.definitions.size());
            queueCreateInfos.reserve(queues.definitions.size());

            for (auto& definition : queues.definitions) {
                queuePriorities.emplace_back(definition.members, 1.0);
                queueCreateInfos.emplace_back(VkDeviceQueueCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .queueFamilyIndex = definition.index,
                    .queueCount = definition.members,
                    .pQueuePriorities = queuePriorities.back().data(),
                });
            }

            constexpr const char* Swapchain = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

            for (auto& extension : extensions) {
                if (std::string_view(extension.extensionName) == Swapchain) {
                    swapchainSupported = true;
                    selectedExtensions.emplace_back(extension.extensionName);
                }
            }

            if (features.swapchains && !swapchainSupported) {
                std::printf("error: presentation is required but is unsupported by the selected physical device\n");
                std::exit(1);
            }

            VkDeviceCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueCreateInfoCount = static_cast<std::uint32_t>(queueCreateInfos.size()),
                .pQueueCreateInfos = queueCreateInfos.data(),
                .enabledLayerCount = 0,
                .ppEnabledLayerNames = nullptr,
                .enabledExtensionCount = static_cast<std::uint32_t>(selectedExtensions.size()),
                .ppEnabledExtensionNames = selectedExtensions.data(),
                .pEnabledFeatures = nullptr,
            };

            VkResult result = vkCreateDevice(physicalDevice_->handle(), &createInfo, nullptr, &device_);

            if (result != VK_SUCCESS) {
                std::printf("error: failed to create device\n");
                std::exit(1);
            }

            physicalDevice_->acquire();

            auto queueFamilies = physicalDevice_->queueFamilies();

            for (auto& family : queueFamilies_) {
                queues_.reserve(family.members);

                for (std::uint32_t i = 0; i < family.members; ++i) {
                    VkQueue queue;

                    vkGetDeviceQueue(device_, family.index, i, &queue);

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

        ~Device() {
            if (device_) {
                vkDestroyDevice(device_, nullptr);
                physicalDevice_->release();
                device_ = nullptr;
            }
        }

        [[nodiscard]] VkDevice handle() const {
            return device_;
        }

        [[nodiscard]] PhysicalDevice& physicalDevice() {
            return *physicalDevice_;
        }

        [[nodiscard]] Instance& instance() {
            return *instance_;
        }

        [[nodiscard]] const Instance& instance() const {
            return *instance_;
        }

        [[nodiscard]] const PhysicalDevice& physicalDevice() const {
            return *physicalDevice_;
        }

        [[nodiscard]] std::span<Queue> getQueues(std::uint32_t familyIndex) {
            std::int64_t offset = 0;

            for (std::size_t i = 0; i < familyIndex; ++i) {
                offset += queueFamilies_[i].members;
            }

            return {queues_.begin() + offset, queueFamilies_[familyIndex].members};
        }

        [[nodiscard]] std::span<const Queue> getQueues(std::uint32_t familyIndex) const {
            std::int64_t offset = 0;

            for (std::size_t i = 0; i < familyIndex; ++i) {
                offset += queueFamilies_[i].members;
            }

            return {queues_.begin() + offset, queueFamilies_[familyIndex].members};
        }

    private:
        using Extensions = std::vector<VkExtensionProperties>;
        using Names = std::vector<const char*>;

        VkDevice device_ = nullptr;

        Instance* instance_;
        PhysicalDevice* physicalDevice_;

        std::vector<QueueFamilyDefinition> queueFamilies_;
        std::vector<Queue> queues_;

        [[nodiscard]] Extensions getExtensions() {
            std::uint32_t extensionCount = 0;

            VkResult result = vkEnumerateDeviceExtensionProperties(physicalDevice_->handle(), nullptr, &extensionCount, nullptr);

            if (result != VK_SUCCESS) {
                std::printf("error: failed to enumerate device extensions\n");
                std::exit(1);
            }

            std::vector<VkExtensionProperties> extensions(extensionCount);

            result = vkEnumerateDeviceExtensionProperties(physicalDevice_->handle(), nullptr, &extensionCount, extensions.data());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to enumerate device extensions\n");
                std::exit(1);
            }

            return extensions;
        }
    };
}