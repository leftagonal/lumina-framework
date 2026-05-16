#pragma once

#include "instance.hpp"
#include "window_manager.hpp"

namespace lumina::system {
    WindowManager::WindowManager(Instance& instance, const WindowManagerEventSockets& sockets) {
        create(instance, sockets);
    }

    WindowManager::~WindowManager() {
        destroy();
    }

    WindowManager::WindowManager(WindowManager&& other) noexcept
        : instance_(other.instance_), windows_(std::move(other.windows_)), freeList_(std::move(other.freeList_)) {
        other.instance_ = nullptr;
    }

    WindowManager& WindowManager::operator=(WindowManager&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        instance_ = other.instance_;
        windows_ = std::move(other.windows_);
        freeList_ = std::move(other.freeList_);

        other.instance_ = nullptr;

        return *this;
    }

    WindowManager::operator bool() const {
        return valid();
    }

    void WindowManager::create(Instance& instance, const WindowManagerEventSockets& sockets) {
        if (valid()) {
            return;
        }

        sockets_ = sockets;
        instance_ = &instance;
    }

    void WindowManager::destroy() {
        if (!valid()) {
            return;
        }

        for (std::size_t i = 0; i < windows_.size(); ++i) {
            auto& window = windows_[i];

            if (!window) {
                continue;
            }

            if (sockets_.destroySocket) {
                sockets_.destroySocket->attempt({i});
            }

            window.destroy();
        }

        instance_ = nullptr;
        windows_.clear();
        freeList_.clear();
    }

    bool WindowManager::valid() const {
        return instance_ != nullptr;
    }

    WindowHandle WindowManager::insert(const WindowInfo& info, const WindowEventSockets& sockets) {
        WindowHandle handle;

        if (freeList_.empty()) {
            handle = windows_.size();

            windows_.emplace_back(handle, info, sockets);
        } else {
            handle = freeList_.back();
            freeList_.pop_back();

            windows_[handle.id()].create(handle, info, sockets);
        }

        if (sockets_.createSocket) {
            sockets_.createSocket->attempt({handle});
        }

        return handle;
    }

    void WindowManager::remove(const WindowHandle& handle) {
        if (!contains(handle)) {
            return;
        }

        if (sockets_.destroySocket) {
            sockets_.destroySocket->attempt({handle});
        }

        windows_[handle.id()].destroy();
        freeList_.emplace_back(handle.id());
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

    Instance& WindowManager::instance() {
        return *instance_;
    }

    const Instance& WindowManager::instance() const {
        return *instance_;
    }

    std::vector<Window>& WindowManager::windows() {
        return windows_;
    }

    const std::vector<Window>& WindowManager::windows() const {
        return windows_;
    }

    std::size_t WindowManager::count() const {
        return windows_.size() - freeList_.size();
    }

    bool WindowManager::empty() const {
        return count() == 0;
    }

    WindowManagerEventSockets& WindowManager::sockets() {
        return sockets_;
    }

    const WindowManagerEventSockets& WindowManager::sockets() const {
        return sockets_;
    }
}
