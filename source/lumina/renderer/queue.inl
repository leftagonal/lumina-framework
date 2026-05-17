#pragma once

#include "logical_device.hpp"
#include "queue.hpp"

namespace lumina::renderer {
    Queue::~Queue() {
        destroy();
    }

    Queue::Queue(LogicalDevice& logicalDevice, QueueIntent intent, std::size_t index) {
        create(logicalDevice, intent, index);
    }

    Queue::Queue(Queue&& other) noexcept
        : queue_(other.queue_), selection_(other.selection_),
          definition_(other.definition_), intent_(other.intent_), index_(other.index_) {
        other.queue_ = nullptr;
    }

    Queue& Queue::operator=(Queue&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        queue_ = other.queue_;
        selection_ = other.selection_;
        definition_ = other.definition_;
        intent_ = other.intent_;
        index_ = other.index_;

        other.queue_ = nullptr;

        return *this;
    }

    Queue::operator bool() const {
        return valid();
    }

    void Queue::create(LogicalDevice& logicalDevice, QueueIntent intent, std::size_t index) {
        if (valid()) {
            return;
        }

        auto entry = accessors::LogicalDeviceAccessor::queueEntry(logicalDevice, intent, index);

        queue_ = accessors::LogicalDeviceAccessor::queue(logicalDevice, intent, index);

        selection_ = entry.selection;
        definition_ = entry.definition;

        intent_ = intent;
        index_ = index;
    }

    void Queue::destroy() {
        if (!valid()) {
            return;
        }

        queue_ = nullptr;
    }

    bool Queue::valid() const {
        return queue_ != nullptr;
    }

    QueueIntent Queue::intent() const {
        return intent_;
    }

    std::size_t Queue::index() const {
        return index_;
    }

    QueueFamilySelection Queue::selection() const {
        return selection_;
    }

    QueueFamilyDefinition Queue::definition() const {
        return definition_;
    }
}
