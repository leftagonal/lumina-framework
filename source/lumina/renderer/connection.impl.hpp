#pragma once

#include "connection.hpp"

namespace lumina::renderer {
    inline Connection::Connection(Instance& instance)
        : presenter_(instance), instance_(&instance) {
    }

    inline WindowHandle Connection::open(const ConnectionInfo& connectionInfo) {
        WindowHandle window = presenter_.create(connectionInfo.initialWindow);

        device_.connect(presenter_, connectionInfo.deviceRequirements);

        return window;
    }

    inline void Connection::close() {
        presenter_.reset();
        device_.disconnect();
    }

    inline WindowHandle Connection::createWindow(const WindowInfo& windowInfo) {
        return presenter_.create(windowInfo);
    }

    inline void Connection::destroyWindow(WindowHandle handle) {
        presenter_.destroy(handle);
    }

    inline WindowAttributes Connection::windowAttributes(WindowHandle handle) const {
        return presenter_.attributes(handle);
    }
}