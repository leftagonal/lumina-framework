#pragma once

#include "../meta/resource.hpp"
#include "../systems/device.hpp"

namespace lumina::render {
    /**
     * @brief Represents a CPU<-GPU synchronisation object.
     *
     */
    class Fence : public Resource<VkFence> {
    public:
        Fence() = default;

        /**
         * @brief Construct a new fence.
         *
         * @param device The device that the fence will operate with.
         * @param startSignalled Whether the fence should start as signalled.
         */
        Fence(Device& device, bool startSignalled = false)
            : device_(&device) {
            VkFenceCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .pNext = nullptr,
                .flags = startSignalled ? VK_FENCE_CREATE_SIGNALED_BIT : 0u,
            };

            VkResult result = vkCreateFence(*device_, &createInfo, nullptr, &resource());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to create fence\n");
                std::exit(1);
            }
        }

        /**
         * @brief Destroy the fence.
         *
         */
        ~Fence() {
            destroy();
        }

        Fence(Fence&&) noexcept = default;

        Fence& operator=(Fence&&) noexcept = default;

        /**
         * @brief Construct a new fence.
         *
         * @param device The device that the fence will operate with.
         * @param startSignalled Whether the fence should start as signalled.
         */
        void create(Device& device, bool startSignalled = false) {
            if (*this) {
                return;
            }

            device_ = &device;

            VkFenceCreateInfo createInfo = {
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .pNext = nullptr,
                .flags = startSignalled ? VK_FENCE_CREATE_SIGNALED_BIT : 0u,
            };

            VkResult result = vkCreateFence(*device_, &createInfo, nullptr, &resource());

            if (result != VK_SUCCESS) {
                std::printf("error: failed to create fence\n");
                std::exit(1);
            }
        }

        /**
         * @brief Destroy the fence.
         *
         */
        void destroy() {
            if (!*this) {
                return;
            }

            vkDestroyFence(*device_, resource(), nullptr);

            device_ = nullptr;

            invalidate();
        }

        [[nodiscard]] bool status() const {
            VkResult result = vkGetFenceStatus(*device_, resource());

            switch (result) {
                case VK_SUCCESS: {
                    return true;
                }
                case VK_NOT_READY: {
                    return false;
                }
                default: {
                    std::printf("error: failed to query fence status\n");
                    std::exit(1);
                }
            }
        }

    private:
        Device* device_ = nullptr;
    };
}