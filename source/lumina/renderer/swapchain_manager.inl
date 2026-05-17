#pragma once

#include "logical_device.hpp"
#include "swapchain_manager.hpp"

namespace lumina::renderer {
    SwapchainManager::~SwapchainManager() {
        destroy();
    }

    SwapchainManager::SwapchainManager(LogicalDevice& logicalDevice, SurfaceManager& surfaceManager) {
        create(logicalDevice, surfaceManager);
    }

    SwapchainManager::SwapchainManager(SwapchainManager&& other) noexcept
        : logicalDevice_(other.logicalDevice_), surfaceManager_(other.surfaceManager_), swapchains_(std::move(other.swapchains_)) {
        other.logicalDevice_ = nullptr;
        other.surfaceManager_ = nullptr;
    }

    SwapchainManager& SwapchainManager::operator=(SwapchainManager&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        logicalDevice_ = other.logicalDevice_;
        surfaceManager_ = other.surfaceManager_;
        swapchains_ = std::move(other.swapchains_);

        other.logicalDevice_ = nullptr;
        other.surfaceManager_ = nullptr;

        return *this;
    }

    SwapchainManager::operator bool() const {
        return valid();
    }

    void SwapchainManager::create(LogicalDevice& logicalDevice, SurfaceManager& surfaceManager) {
        if (valid()) {
            return;
        }

        logicalDevice_ = &logicalDevice;
        surfaceManager_ = &surfaceManager;

        auto sockets = surfaceManager_->sockets();

        destroyToken_ = sockets.destroySocket->connect<&SwapchainManager::onSurfaceDestroy>(*this);
    }

    void SwapchainManager::destroy() {
        if (!valid()) {
            return;
        }

        auto sockets = surfaceManager_->sockets();

        sockets.destroySocket->disconnect(destroyToken_);

        logicalDevice_ = nullptr;
        surfaceManager_ = nullptr;
        swapchains_.clear();
    }

    void SwapchainManager::insert(const system::WindowHandle& target, const SwapchainInfo& info, const SwapchainQueueInfo& queueInfo) {
        if (target.id() >= swapchains_.size()) {
            swapchains_.resize(target.id() + 1);
        }

        auto& swapchain = swapchains_[target.id()];

        swapchain.create(*logicalDevice_, *surfaceManager_, info, queueInfo, target);
    }

    bool SwapchainManager::valid() const {
        return logicalDevice_ != nullptr;
    }

    LogicalDevice& SwapchainManager::logicalDevice() {
        return *logicalDevice_;
    }

    const LogicalDevice& SwapchainManager::logicalDevice() const {
        return *logicalDevice_;
    }

    void SwapchainManager::onSurfaceDestroy(const SurfaceManagerSurfaceDestroyEvent& event) {
        if (event.target.id() >= swapchains_.size()) {
            return;
        }

        auto& swapchain = swapchains_[event.target.id()];

        swapchain.destroy();
    }
}
