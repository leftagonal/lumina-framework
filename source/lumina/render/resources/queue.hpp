#pragma once

#include "../meta/resource.hpp"

#include <vulkan/vulkan.h>

#include <cstdint>

namespace lumina::render {
    /**
     * @brief Defines the family location of a given queue.
     *
     */
    struct QueueAssignment {
        std::uint32_t familyIndex;
        std::uint32_t queueIndex;
    };

    /**
     * @brief Defines the capabilities of a given queue family.
     *
     */
    struct QueueFamilyFeatures {
        bool graphics;
        bool transfer;
        bool compute;
    };

    /**
     * @brief Defines a queue family's index and members.
     *
     */
    struct QueueFamilyDefinition {
        std::uint32_t index;
        std::uint32_t members;
    };

    /**
     * @brief Represents an entire single queue family.
     *
     */
    struct QueueFamily {
        QueueFamilyDefinition definition;
        QueueFamilyFeatures features;
    };

    class Queue : public Resource<VkQueue> {
    public:
        /**
         * @brief Construct a new queue.
         *
         * @param queue The provided queue handle.
         * @param assignment The queue's family information.
         * @param features The queue's supported capabilities.
         */
        Queue(VkQueue queue, const QueueAssignment& assignment, const QueueFamilyFeatures& features)
            : Resource(queue), assignment_(assignment), features_(features) {
        }

        /**
         * @brief Destroy the queue.
         *
         */
        ~Queue() {
            assignment_ = {};
            features_ = {};

            invalidate();
        }

        Queue(Queue&&) noexcept = default;

        Queue& operator=(Queue&&) noexcept = default;

        /**
         * @brief Provides the queue's assignment information.
         *
         * @return QueueAssignment The queue's assignment.
         */
        [[nodiscard]] QueueAssignment assignment() const {
            return assignment_;
        }

        /**
         * @brief Provides the queue's capabilities.
         *
         * @return QueueFamilyFeatures The queue's family capabilities.
         */
        [[nodiscard]] QueueFamilyFeatures features() const {
            return features_;
        }

    private:
        QueueAssignment assignment_;
        QueueFamilyFeatures features_;
    };
}