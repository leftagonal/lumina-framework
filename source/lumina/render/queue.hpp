#pragma once

#include <cstdint>
#include <vulkan/vulkan.h>

namespace lumina::render {
    struct QueueAssignment {
        std::uint32_t familyIndex;
        std::uint32_t queueOffset;
    };

    class Queue {
    public:
        Queue(VkQueue queue, QueueAssignment assignment)
            : queue_(queue), assignment_(assignment) {
        }

        [[nodiscard]] VkQueue handle() const {
            return queue_;
        }

        [[nodiscard]] QueueAssignment assignment() const {
            return assignment_;
        }

    private:
        VkQueue queue_;
        QueueAssignment assignment_;
    };
}