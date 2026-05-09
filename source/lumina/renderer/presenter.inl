#pragma once

#include "device.hpp"
#include "instance.hpp"
#include "presenter.hpp"

#include <lumina/meta/console.hpp>
#include <lumina/meta/exceptions.hpp>

namespace lumina::renderer::subsystems {
    inline Presenter::Presenter(Instance& instance)
        : debugging_(instance.debugging()), instance_(&instance) {
    }

    inline Presenter::~Presenter() {
        reset();
    }

    inline void Presenter::reset() {
        if (!windows_.empty()) {
            for (std::size_t i = 0; i < windows_.size(); ++i) {
                destroy(i);
            }

            windows_.clear();

            meta::logDebug(debugging_, "presenter destroyed");
        }
    }

    inline WindowHandle Presenter::create(const WindowInfo& info) {
        if (info.status == WindowStatus::Inactive) {
            throw meta::Exception("window should not be constructed as inactive");
        }

        if (info.extent.width == 0 || info.extent.height == 0) {
            throw meta::Exception("no window extent axis should equal zero");
        }

        std::size_t id = windows_.size();

        WindowCache* window = nullptr;
        VkSurfaceKHR* surface = nullptr;

        if (!freeList_.empty()) {
            id = freeList_.back();
            freeList_.pop_back();

            window = &windows_[id];
            surface = &surfaces_[id];
        } else {
            window = &windows_.emplace_back(WindowCache{
                .status = info.status,
                .lastStatus = info.status,
                .focus = WindowFocus::Focused,
                .title = std::string(info.title),
                .extent = info.extent,
                .resizable = info.resizable,
                .resource = nullptr,
            });

            surface = &surfaces_.emplace_back(nullptr);

            swapchains_.emplace_back(nullptr);
        }

        glfwWindowHint(GLFW_RESIZABLE, info.resizable);

        window->resource = glfwCreateWindow(
            static_cast<int>(info.extent.width),
            static_cast<int>(info.extent.height),
            info.title.data(),
            nullptr,
            nullptr);

        meta::logDebug(debugging_, "created window (handle: {}):", id);
        meta::logDebugListElement(debugging_, 1, "title: {}", window->title);
        meta::logDebugListElement(debugging_, 1, "extent: {}x{}", window->extent.width, window->extent.height);
        meta::logDebugListElement(debugging_, 1, "resizable: {}", window->resizable);

        window->userPointer = std::make_unique<WindowUserPointerData>(id, this);

        glfwSetWindowUserPointer(window->resource, window->userPointer.get());
        glfwSetFramebufferSizeCallback(window->resource, resizeCallback);
        glfwSetWindowCloseCallback(window->resource, closeCallback);
        glfwSetWindowIconifyCallback(window->resource, iconifyCallback);
        glfwSetWindowFocusCallback(window->resource, focusCallback);

        switch (window->status) {
            case WindowStatus::Iconified: {
                glfwIconifyWindow(window->resource);

                break;
            }
            default: {
                break;
            }
        }

        VkInstance instance = accessors::InstanceAccessor::instance(*instance_);
        VkResult result = glfwCreateWindowSurface(instance, window->resource, nullptr, surface);

        if (result != VK_SUCCESS) {
            throw meta::Exception("window surface creation failed");
        }

        return id;
    }

    inline void Presenter::destroy(const WindowHandle& handle) {
        if (!handle.valid() || handle.id() >= windows_.size()) {
            return;
        }

        WindowCache& window = windows_[handle.id()];
        VkSurfaceKHR& surface = surfaces_[handle.id()];
        VkSwapchainKHR& swapchain = swapchains_[handle.id()];

        VkInstance instance = accessors::InstanceAccessor::instance(*instance_);

        if (swapchain != nullptr) {
            // vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks *pAllocator)

            swapchain = nullptr;
        }

        if (surface != nullptr) {
            vkDestroySurfaceKHR(instance, surface, nullptr);

            surface = nullptr;
        }

        if (window.resource != nullptr) {
            glfwDestroyWindow(window.resource);

            meta::logDebug(
                debugging_,
                "destroyed window (handle: {})",
                handle.id());

            window.resource = nullptr;
        }

        freeList_.emplace_back(handle.id());
    }

    inline void Presenter::resizeCallback(GLFWwindow* window, int width, int height) {
        void* pointer = glfwGetWindowUserPointer(window);

        if (pointer == nullptr) {
            return;
        }

        WindowUserPointerData& data = *static_cast<WindowUserPointerData*>(pointer);
        WindowCache& cache = data.presenter->windows_[data.handle.id()];

        cache.extent.width = static_cast<std::uint32_t>(width);
        cache.extent.height = static_cast<std::uint32_t>(height);

        meta::logDebug(
            data.presenter->debugging_,
            "window resized (handle: {}, {}x{})",
            data.handle.id(),
            cache.extent.width,
            cache.extent.height);
    }

    inline void Presenter::closeCallback(GLFWwindow* window) {
        void* pointer = glfwGetWindowUserPointer(window);

        if (pointer == nullptr) {
            return;
        }

        WindowUserPointerData& data = *static_cast<WindowUserPointerData*>(pointer);
        WindowCache& cache = data.presenter->windows_[data.handle.id()];

        if (cache.status == WindowStatus::Inactive) {
            return;
        }

        // window should become "closed"
        cache.lastStatus = cache.status;
        cache.status = WindowStatus::Inactive;

        if (data.presenter->debugging_) {
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
                data.presenter->debugging_,
                "window state change: {} -> {} (handle: {})",
                lastStatus,
                currentStatus,
                data.handle.id());
        }
    }

    inline void Presenter::iconifyCallback(GLFWwindow* window, int status) {
        void* pointer = glfwGetWindowUserPointer(window);

        if (pointer == nullptr) {
            return;
        }

        WindowUserPointerData& data = *static_cast<WindowUserPointerData*>(pointer);
        WindowCache& cache = data.presenter->windows_[data.handle.id()];

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

        if (data.presenter->debugging_) {
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
                data.presenter->debugging_,
                "window state change: {} -> {} (handle: {})",
                lastStatus,
                currentStatus,
                data.handle.id());
        }
    }

    inline void Presenter::focusCallback(GLFWwindow* window, int status) {
        void* pointer = glfwGetWindowUserPointer(window);

        if (pointer == nullptr) {
            return;
        }

        WindowUserPointerData& data = *static_cast<WindowUserPointerData*>(pointer);
        WindowCache& cache = data.presenter->windows_[data.handle.id()];

        // you can't focus an inactive or iconified window
        if (cache.status == WindowStatus::Inactive || cache.status == WindowStatus::Iconified) {
            return;
        }

        if (status) {
            cache.focus = WindowFocus::Focused;
        } else {
            cache.focus = WindowFocus::Unfocused;
        }

        if (data.presenter->debugging_) {
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
                data.presenter->debugging_,
                "window focus change: {} -> {} (handle: {})",
                lastFocus,
                currentFocus,
                data.handle.id());
        }
    }
}