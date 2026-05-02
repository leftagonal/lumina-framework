#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>

namespace lumina::render {
    struct QueueAssignment {
        std::uint32_t familyIndex;
        std::uint32_t queueIndex;
    };

    struct QueueFamilyFeatures {
        bool graphics;
        bool transfer;
        bool compute;
    };

    struct QueueFamilyDefinition {
        std::uint32_t index;
        std::uint32_t members;
    };

    struct QueueFamily {
        QueueFamilyDefinition definition;
        QueueFamilyFeatures features;
    };

    class Queue {
    public:
        Queue(VkQueue queue, const QueueAssignment& assignment, const QueueFamilyFeatures& features)
            : queue_(queue), assignment_(assignment), features_(features) {
        }

        [[nodiscard]] VkQueue handle() const {
            return queue_;
        }

        [[nodiscard]] QueueAssignment assignment() const {
            return assignment_;
        }

        [[nodiscard]] QueueFamilyFeatures features() const {
            return features_;
        }

    private:
        VkQueue queue_;
        QueueAssignment assignment_;
        QueueFamilyFeatures features_;
    };
}