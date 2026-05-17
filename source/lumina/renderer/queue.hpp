#pragma once

#include <lumina/core/vulkan.hpp>

namespace lumina::renderer {
    class LogicalDevice;

    struct QueueFamilySelection {
        std::uint32_t familyIndex = 0;
        std::uint32_t queueIndex = 0;
    };

    struct QueueFamilyDefinition {
        std::uint32_t familyIndex = 0;
        std::uint32_t queueCount = 0;
    };

    enum class QueueIntent {
        Present,
        Graphics,
        Compute,
        Transfer,
    };

    class Queue final {
    public:
        Queue() = default;
        ~Queue();

        Queue(LogicalDevice& logicalDevice, QueueIntent intent, std::size_t index);

        Queue(const Queue&) = delete;
        Queue(Queue&&) noexcept;

        Queue& operator=(const Queue&) = delete;
        Queue& operator=(Queue&&) noexcept;

        explicit operator bool() const;

        void create(LogicalDevice& logicalDevice, QueueIntent intent, std::size_t index);
        void destroy();

        [[nodiscard]] bool valid() const;
        [[nodiscard]] QueueIntent intent() const;
        [[nodiscard]] std::size_t index() const;

        [[nodiscard]] QueueFamilySelection selection() const;
        [[nodiscard]] QueueFamilyDefinition definition() const;

    private:
        VkQueue queue_ = nullptr;

        QueueFamilySelection selection_ = {};
        QueueFamilyDefinition definition_ = {};

        QueueIntent intent_;
        std::size_t index_;
    };
}
