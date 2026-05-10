#pragma once

#include "display_server.hpp"
#include "instance.hpp"
#include "logical_device.hpp"

#include <lumina/meta/console.hpp>
#include <lumina/meta/exceptions.hpp>

namespace lumina::renderer {
    inline DisplayServer::DisplayServer(Instance& instance)
        : validation_(instance.validation()), instance_(&instance) {
    }

    inline DisplayServer::~DisplayServer() {
        reset();
    }

    inline WindowHandle DisplayServer::create(const WindowInfo& info) {
        bool widthIsZero = info.extent.width == 0;
        bool heightIsZero = info.extent.height == 0;

        meta::assert(!widthIsZero && !heightIsZero, "no window extent axis should equal zero");

        std::size_t id = windows_.size();

        WindowCache* cache = nullptr;
        GLFWwindow** window = nullptr;
        VkSurfaceKHR* surface = nullptr;

        if (!freeList_.empty()) {
            id = freeList_.back();
            freeList_.pop_back();

            window = &windows_[id];
            surface = &surfaces_[id];
        } else {
            WindowCache config = {
                .status = WindowStatus::Active,
                .lastStatus = WindowStatus::Active,
                .focus = WindowFocus::Focused,
                .title = std::string(info.title),
                .extent = info.extent,
                .resizable = info.resizable,
            };

            window = &windows_.emplace_back();
            cache = &caches_.emplace_back(config);
            surface = &surfaces_.emplace_back(nullptr);
            swapchains_.emplace_back(nullptr);
        }

        glfwWindowHint(GLFW_RESIZABLE, info.resizable);

        int width = static_cast<int>(info.extent.width);
        int height = static_cast<int>(info.extent.height);
        const char* title = info.title.data();

        *window = glfwCreateWindow(width, height, title, nullptr, nullptr);

        meta::logDebug(validation_, "created window (handle: {}):", id);
        meta::logDebugListElement(validation_, 1, "title: {}", cache->title);
        meta::logDebugListElement(validation_, 1, "extent: {}x{}", cache->extent.width, cache->extent.height);
        meta::logDebugListElement(validation_, 1, "resizable: {}", cache->resizable);

        glfwSetWindowUserPointer(*window, new WindowBinding(id, *this));
        glfwSetFramebufferSizeCallback(*window, resizeCallback);
        glfwSetWindowCloseCallback(*window, closeCallback);
        glfwSetWindowIconifyCallback(*window, iconifyCallback);
        glfwSetWindowFocusCallback(*window, focusCallback);

        VkInstance instance = accessors::InstanceAccessor::instance(*instance_);
        VkResult result = glfwCreateWindowSurface(instance, *window, nullptr, surface);

        if (result != VK_SUCCESS) {
            throw meta::Exception("window surface creation failed");
        }

        return id;
    }

    inline void DisplayServer::destroy(const WindowHandle& handle) {
        if (!handle.valid() || handle.id() >= windows_.size()) {
            return;
        }

        auto index = handle.id();

        GLFWwindow*& window = windows_[index];
        VkSurfaceKHR& surface = surfaces_[index];
        VkSwapchainKHR& swapchain = swapchains_[index];

        VkInstance instance = accessors::InstanceAccessor::instance(*instance_);

        if (swapchain != nullptr) {
            // vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks *pAllocator)

            swapchain = nullptr;
        }

        if (surface != nullptr) {
            vkDestroySurfaceKHR(instance, surface, nullptr);

            surface = nullptr;
        }

        if (window != nullptr) {
            void* pointer = glfwGetWindowUserPointer(window);
            auto* binding = reinterpret_cast<WindowBinding*>(pointer);

            if (binding) {
                delete binding;
                binding = nullptr;
            }

            glfwDestroyWindow(window);

            meta::logDebug(validation_, "destroyed window (handle: {})", handle.id());

            window = nullptr;
        }

        freeList_.emplace_back(handle.id());
    }

    inline void DisplayServer::reset() {
        if (!windows_.empty()) {
            for (std::size_t i = 0; i < windows_.size(); ++i) {
                destroy(i);
            }

            windows_.clear();

            meta::logDebug(validation_, "presenter destroyed");
        }
    }

    inline void DisplayServer::resizeCallback(GLFWwindow* window, int width, int height) {
        void* pointer = glfwGetWindowUserPointer(window);

        if (pointer == nullptr) {
            return;
        }

        WindowBinding& binding = *static_cast<WindowBinding*>(pointer);
        WindowCache& cache = binding.server.caches_[binding.handle.id()];

        cache.extent.width = static_cast<std::uint32_t>(width);
        cache.extent.height = static_cast<std::uint32_t>(height);

        meta::logDebug(
            binding.server.validation_,
            "window resized (handle: {}, {}x{})",
            binding.handle.id(),
            cache.extent.width,
            cache.extent.height);
    }

    inline void DisplayServer::closeCallback(GLFWwindow* window) {
        void* pointer = glfwGetWindowUserPointer(window);

        if (pointer == nullptr) {
            return;
        }

        WindowBinding& binding = *static_cast<WindowBinding*>(pointer);
        WindowCache& cache = binding.server.caches_[binding.handle.id()];

        if (cache.status == WindowStatus::Inactive) {
            return;
        }

        // window should become "closed"
        cache.lastStatus = cache.status;
        cache.status = WindowStatus::Inactive;

        if (!binding.server.validation_) {
            return;
        }

        std::string lastStatus;
        std::string currentStatus = "inactive";

        switch (cache.lastStatus) {
            case WindowStatus::Iconified: {
                lastStatus = "iconified";

                break;
            }
            case WindowStatus::Active: {
                lastStatus = "active";

                break;
            }
            default: {
                break;
            }
        }

        meta::logDebug(
            binding.server.validation_,
            "window state change: {} -> {} (handle: {})",
            lastStatus,
            currentStatus,
            binding.handle.id());
    }

    inline void DisplayServer::iconifyCallback(GLFWwindow* window, int status) {
        void* pointer = glfwGetWindowUserPointer(window);

        if (pointer == nullptr) {
            return;
        }

        WindowBinding& binding = *static_cast<WindowBinding*>(pointer);
        WindowCache& cache = binding.server.caches_[binding.handle.id()];

        // you can't iconify or activate an inactive window
        if (cache.status == WindowStatus::Inactive) {
            return;
        }

        if (status) {
            cache.lastStatus = cache.status;
            cache.status = WindowStatus::Iconified;
        } else {
            cache.status = cache.lastStatus;
        }

        if (!binding.server.validation_) {
            return;
        }

        std::string lastStatus;
        std::string currentStatus;

        if (cache.status == WindowStatus::Active) {
            lastStatus = "iconified";
            currentStatus = "active";
        } else {
            lastStatus = "active";
            currentStatus = "iconified";
        }

        meta::logDebug(
            binding.server.validation_,
            "window state change: {} -> {} (handle: {})",
            lastStatus,
            currentStatus,
            binding.handle.id());
    }

    inline void DisplayServer::focusCallback(GLFWwindow* window, int status) {
        void* pointer = glfwGetWindowUserPointer(window);

        if (pointer == nullptr) {
            return;
        }

        WindowBinding& binding = *static_cast<WindowBinding*>(pointer);
        WindowCache& cache = binding.server.caches_[binding.handle.id()];

        // you can't focus an inactive or iconified window
        if (cache.status == WindowStatus::Inactive || cache.status == WindowStatus::Iconified) {
            return;
        }

        if (status) {
            cache.focus = WindowFocus::Focused;
        } else {
            cache.focus = WindowFocus::Unfocused;
        }

        if (!binding.server.validation_) {
            return;
        }

        std::string lastFocus;
        std::string currentFocus;

        if (cache.focus == WindowFocus::Focused) {
            lastFocus = "unfocused";
            currentFocus = "focused";
        } else {
            lastFocus = "focused";
            currentFocus = "unfocused";
        }

        meta::logDebug(
            binding.server.validation_,
            "window focus change: {} -> {} (handle: {})",
            lastFocus,
            currentFocus,
            binding.handle.id());
    }
}
