#pragma once

#include "../meta/resource.hpp"
#include "../systems/device.hpp"

namespace lumina::render {
    /**
     * @brief Represents a timeline-based CPU<->GPU synchronisation object.
     *
     */
    class TimelineSemaphore : public Resource<VkSemaphore> {
    public:
        TimelineSemaphore() = default;

        /**
         * @brief Construct a new timeline semaphore.
         *
         * @param device The device that the timeline semaphore will operate on.
         * @param value The initial value of the timeline semaphore.
         */
        TimelineSemaphore(Device& device, std::uint64_t value)
            : device_(&device) {
            VkSemaphoreTypeCreateInfo typeInfo = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
                .pNext = nullptr,
                .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
                .initialValue = value,
            };

            VkSemaphoreCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = &typeInfo,
                .flags = 0,
            };

            VkResult result = vkCreateSemaphore(*device_, &createInfo, nullptr, &resource());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to create semaphore\n");
                std::exit(1);
            }
        }

        /**
         * @brief Destroy the timeline semaphore.
         *
         */
        ~TimelineSemaphore() {
            destroy();
        }

        TimelineSemaphore(TimelineSemaphore&&) noexcept = default;

        TimelineSemaphore& operator=(TimelineSemaphore&&) noexcept = default;

        /**
         * @brief Construct a new timeline semaphore.
         *
         * @param device The device that the timeline semaphore will operate on.
         * @param value The initial value of the timeline semaphore.
         */
        void create(Device& device, std::uint64_t value) {
            if (*this) {
                return;
            }

            device_ = &device;

            VkSemaphoreTypeCreateInfo typeInfo = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
                .pNext = nullptr,
                .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
                .initialValue = value,
            };

            VkSemaphoreCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = &typeInfo,
                .flags = 0,
            };

            VkResult result = vkCreateSemaphore(*device_, &createInfo, nullptr, &resource());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to create semaphore\n");
                std::exit(1);
            }
        }

        /**
         * @brief Destroy the timeline semaphore.
         *
         */
        void destroy() {
            if (!*this) {
                return;
            }

            vkDestroySemaphore(*device_, resource(), nullptr);

            device_ = nullptr;

            invalidate();
        }

        std::uint64_t value() const {
            std::uint64_t value = 0;

            VkResult result = vkGetSemaphoreCounterValue(*device_, resource(), &value);

            if (result != VK_SUCCESS) {
                std::printf("error: failed to query timeline semaphore value\n");
                std::exit(1);
            }

            return value;
        }

        void step(std::uint64_t value) {
            VkSemaphoreSignalInfo signalInfo = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
                .pNext = nullptr,
                .semaphore = resource(),
                .value = value,
            };

            VkResult result = vkSignalSemaphore(*device_, &signalInfo);

            if (result != VK_SUCCESS) {
                std::printf("error: failed to increment timeline semaphore\n");
                std::exit(1);
            }
        }

    private:
        Device* device_ = nullptr;
    };
}