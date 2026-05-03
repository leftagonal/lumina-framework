#pragma once

#include "../meta/resource.hpp"
#include "../systems/device.hpp"

namespace lumina::render {
    /**
     * @brief Represents a GOU<->GPU synchronisation object.
     *
     */
    class Semaphore : public Resource<VkSemaphore> {
    public:
        Semaphore() = default;

        /**
         * @brief Construct a new semaphore.
         *
         * @param device The device that the semaphore will operate on.
         */
        Semaphore(Device& device)
            : device_(&device) {
            VkSemaphoreCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
            };

            VkResult result = vkCreateSemaphore(*device_, &createInfo, nullptr, &resource());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to create semaphore\n");
                std::exit(1);
            }
        }

        /**
         * @brief Destroy the semaphore.
         *
         */
        ~Semaphore() {
            destroy();
        }

        Semaphore(Semaphore&&) noexcept = default;

        Semaphore& operator=(Semaphore&&) noexcept = default;

        /**
         * @brief Construct a new semaphore.
         *
         * @param device The device that the semaphore will operate on.
         */
        void create(Device& device) {
            if (*this) {
                return;
            }

            device_ = &device;

            VkSemaphoreCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
            };

            VkResult result = vkCreateSemaphore(*device_, &createInfo, nullptr, &resource());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to create semaphore\n");
                std::exit(1);
            }
        }

        /**
         * @brief Destroy the semaphore.
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

    private:
        Device* device_ = nullptr;
    };
}