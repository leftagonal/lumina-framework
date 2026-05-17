#pragma once

#include "instance.hpp"
#include "surface_manager.hpp"

namespace lumina::renderer {
    SurfaceManager::~SurfaceManager() {
        destroy();
    }

    SurfaceManager::SurfaceManager(Instance& instance, system::WindowManager& windowManager, const SurfaceManagerEventSockets& sockets) {
        create(instance, windowManager, sockets);
    }

    SurfaceManager::SurfaceManager(SurfaceManager&& other) noexcept
        : instance_(other.instance_), windowManager_(other.windowManager_), surfaces_(std::move(other.surfaces_)), sockets_(other.sockets_) {
        other.instance_ = nullptr;
        other.windowManager_ = nullptr;
        other.surfaces_.clear();

        other.sockets_.destroySocket.disconnect();
    }

    SurfaceManager& SurfaceManager::operator=(SurfaceManager&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        instance_ = other.instance_;
        windowManager_ = other.windowManager_;
        surfaces_ = std::move(other.surfaces_);
        sockets_ = other.sockets_;

        other.instance_ = nullptr;
        other.windowManager_ = nullptr;
        other.surfaces_.clear();

        other.sockets_.destroySocket.disconnect();

        return *this;
    }

    SurfaceManager::operator bool() const {
        return valid();
    }

    void SurfaceManager::create(Instance& instance, system::WindowManager& windowManager, const SurfaceManagerEventSockets& sockets) {
        if (valid()) {
            return;
        }

        instance_ = &instance;
        windowManager_ = &windowManager;
        sockets_ = sockets;

        createAll();

        auto windowSockets = windowManager_->sockets();

        createToken_ = windowSockets.createSocket->connect<&SurfaceManager::onWindowCreate>(*this);
        destroyToken_ = windowSockets.destroySocket->connect<&SurfaceManager::onWindowDestroy>(*this);
    }

    void SurfaceManager::destroy() {
        if (!valid()) {
            return;
        }

        auto sockets = windowManager_->sockets();

        sockets.createSocket->disconnect(createToken_);
        sockets.destroySocket->disconnect(destroyToken_);

        for (auto& surface : surfaces_) {
            if (surface && sockets_.destroySocket) {
                sockets_.destroySocket->trigger({surface.window().handle()});
            }
        }

        instance_ = nullptr;
        windowManager_ = nullptr;
        surfaces_.clear();
        sockets_.destroySocket.disconnect();
    }

    bool SurfaceManager::valid() const {
        return instance_ != nullptr && windowManager_ != nullptr;
    }

    bool SurfaceManager::empty() const {
        return windowManager_->empty();
    }

    std::vector<Surface>& SurfaceManager::surfaces() {
        return surfaces_;
    }

    const std::vector<Surface>& SurfaceManager::surfaces() const {
        return surfaces_;
    }

    Surface& SurfaceManager::get(const system::WindowHandle& windowHandle) {
        return surfaces_[windowHandle.id()];
    }

    const Surface& SurfaceManager::get(const system::WindowHandle& windowHandle) const {
        return surfaces_[windowHandle.id()];
    }

    bool SurfaceManager::contains(const system::WindowHandle& windowHandle) const {
        return surfaces_.size() > windowHandle.id() && surfaces_[windowHandle.id()];
    }

    SurfaceManagerEventSockets SurfaceManager::sockets() const {
        return sockets_;
    }

    void SurfaceManager::createAll() {
        auto& windows = windowManager_->windows();

        surfaces_.resize(windows.size());

        for (std::size_t i = 0; i < surfaces_.size(); ++i) {
            auto& window = windows[i];

            if (!window) {
                continue;
            }

            surfaces_[i].create(*instance_, *windowManager_, window.handle());
        }
    }

    void SurfaceManager::onWindowCreate(const system::WindowManagerWindowCreateEvent& event) {
        auto& window = windowManager_->get(event.handle.id());

        if (surfaces_.size() <= event.handle.id()) {
            surfaces_.resize(event.handle.id() + 1);
        }

        auto& surface = surfaces_[event.handle.id()];

        if (surface) {
            return;
        }

        surface.create(*instance_, *windowManager_, window.handle());
    }

    void SurfaceManager::onWindowDestroy(const system::WindowManagerWindowDestroyEvent& event) {
        if (surfaces_.size() <= event.handle.id()) {
            return;
        }

        auto& surface = surfaces_[event.handle.id()];

        if (!surface) {
            return;
        }

        if (sockets_.destroySocket) {
            sockets_.destroySocket->trigger({surface.window().handle()});
        }

        surface.destroy();
    }
}
