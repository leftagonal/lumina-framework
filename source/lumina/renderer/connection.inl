#pragma once

#include "connection.hpp"

namespace lumina::renderer {
    inline Connection::Connection(Instance& instance)
        : displayServer_(instance), instance_(&instance) {
    }

    inline OpenResult Connection::open(const ConnectionInfo& connectionInfo) {
        WindowHandle window = displayServer_.create(connectionInfo.pilotWindowInfo);

        logicalDevice_.connect(displayServer_, connectionInfo.deviceRequirements);

        return {
            .pilotWindow = window,
        };
    }

    inline void Connection::close() {
        displayServer_.reset();
        logicalDevice_.disconnect();
    }

    inline WindowHandle Connection::createWindow(const WindowInfo& windowInfo) {
        return displayServer_.create(windowInfo);
    }

    inline void Connection::destroyWindow(WindowHandle handle) {
        displayServer_.destroy(handle);
    }

    inline WindowAttributes Connection::windowAttributes(WindowHandle handle) const {
        return displayServer_.attributes(handle);
    }
}