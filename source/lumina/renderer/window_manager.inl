#pragma once

#include "window_manager.hpp"
#include "instance.hpp"
#include <cstdlib>

namespace lumina::renderer {
    WindowManager::WindowManager(Instance& instance)
    	: instance_(&instance) {
    }

    WindowHandle WindowManager::create(WindowInfo& info) {
    	WindowHandle handle;

        if (freeList_.empty()) {
            handle = windows_.size();

            windows_.emplace_back(handle, *instance_, info);
        } else {
            handle = freeList_.back();
            freeList_.pop_back();

            windows_[handle.id()] = Window(handle, *instance_, info);
        }

        ++aliveCounter_;

        return handle;
    }

    void WindowManager::destroy(const WindowHandle& handle) {
        if (!contains(handle)) {
            return;
        }

        windows_[handle.id()].destroy();
        freeList_.emplace_back(handle.id());

        --aliveCounter_;
    }

    Instance& WindowManager::instance() {
    	return *instance_;
    }

    const Instance& WindowManager::instance() const {
        return *instance_;
    }

    Window& WindowManager::get(const WindowHandle& handle) {
    	return windows_[handle.id()];
    }

    const Window& WindowManager::get(const WindowHandle& handle) const {
        return windows_[handle.id()];
    }

    bool WindowManager::contains(const WindowHandle& handle) const {
    	return windows_.size() > handle.id() && windows_[handle.id()];
    }

    void WindowManager::clear() {
    	windows_.clear();
    }

    std::span<Window> WindowManager::windows() {
    	return windows_;
    }

    std::span<const Window> WindowManager::windows() const {
    	return windows_;
    }

    std::size_t WindowManager::count() const {
    	return aliveCounter_;
    }

    bool WindowManager::empty() const {
    	return aliveCounter_ == 0;
    }
}
