#pragma once

#include "instance.hpp"
#include "surface.hpp"

namespace lumina::renderer {
    Surface::~Surface() {
        destroy();
    }

    Surface::Surface(Instance& instance, system::WindowManager& windowManager, const system::WindowHandle& windowHandle) {
        create(instance, windowManager, windowHandle);
    }

    Surface::Surface(Surface&& other) noexcept
        : instance_(other.instance_), surface_(other.surface_) {
        other.surface_ = nullptr;
        other.instance_ = nullptr;
    }

    Surface& Surface::operator=(Surface&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        instance_ = other.instance_;
        surface_ = other.surface_;

        other.surface_ = nullptr;
        other.instance_ = nullptr;

        return *this;
    }

    Surface::operator bool() const {
        return valid();
    }

    void Surface::create(Instance& instance, system::WindowManager& windowManager, const system::WindowHandle& windowHandle) {
        if (valid()) {
            return;
        }

        instance_ = &instance;

        auto& window = windowManager.get(windowHandle);

        auto windowResource = system::accessors::WindowAccessor::window(window);
        auto instanceResource = accessors::InstanceAccessor::instance(*instance_);

        VkResult result = glfwCreateWindowSurface(instanceResource, windowResource, nullptr, &surface_);

        meta::assert(result == VK_SUCCESS, "failed to create Vulkan window surface: {}", core::Vulkan_errorString(result));
    }

    void Surface::destroy() {
        if (!valid()) {
            return;
        }

        auto instanceHandle = accessors::InstanceAccessor::instance(*instance_);

        vkDestroySurfaceKHR(instanceHandle, surface_, nullptr);

        surface_ = nullptr;
        instance_ = nullptr;
    }

    bool Surface::valid() const {
        return surface_ != nullptr;
    }

    Instance& Surface::instance() {
        return *instance_;
    }

    const Instance& Surface::instance() const {
        return *instance_;
    }
}
