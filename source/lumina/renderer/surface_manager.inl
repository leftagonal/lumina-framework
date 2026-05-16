#pragma once

#include "instance.hpp"
#include "surface_manager.hpp"

namespace lumina::renderer {
    SurfaceManager::~SurfaceManager() {
        destroy();
    }

    SurfaceManager::SurfaceManager(Instance& instance, system::WindowManager& windowManager) {
        create(instance, windowManager);
    }

    SurfaceManager::SurfaceManager(SurfaceManager&& other) noexcept
        : instance_(other.instance_), windowManager_(other.windowManager_), surfaces_(std::move(other.surfaces_)) {
        other.instance_ = nullptr;
        other.windowManager_ = nullptr;
        other.surfaces_.clear();
    }

    SurfaceManager& SurfaceManager::operator=(SurfaceManager&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        instance_ = other.instance_;
        windowManager_ = other.windowManager_;
        surfaces_ = std::move(other.surfaces_);

        other.instance_ = nullptr;
        other.windowManager_ = nullptr;
        other.surfaces_.clear();

        return *this;
    }

    SurfaceManager::operator bool() const {
        return valid();
    }

    void SurfaceManager::create(Instance& instance, system::WindowManager& windowManager) {
        if (valid()) {
            return;
        }

        instance_ = &instance;
        windowManager_ = &windowManager;

        createAll();

        auto sockets = windowManager_->sockets();

        createToken_ = sockets.createSocket->connect<&SurfaceManager::onWindowCreate>(*this);
        destroyToken_ = sockets.destroySocket->connect<&SurfaceManager::onWindowDestroy>(*this);
    }

    void SurfaceManager::destroy() {
        if (!valid()) {
            return;
        }

        auto sockets = windowManager_->sockets();

        sockets.createSocket->disconnect(createToken_);
        sockets.destroySocket->disconnect(destroyToken_);

        instance_ = nullptr;
        windowManager_ = nullptr;
        surfaces_.clear();
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

        surface.destroy();
    }
}
