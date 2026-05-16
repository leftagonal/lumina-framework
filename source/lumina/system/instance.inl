#pragma once

#include <lumina/core/glfw.hpp>

#include "instance.hpp"

namespace lumina::system {
    inline Instance::Instance() {
        glfwSetErrorCallback(core::GLFW_errorCallback);
        glfwInit();

        // stop GLFW from creating windows with an OpenGL context
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }

    inline Instance::~Instance() {
        glfwTerminate();
    }

    inline void Instance::poll() {
        glfwPollEvents();
    }

    inline void Instance::await() {
        glfwWaitEvents();
    }

    inline void Instance::await(double timeout) {
        glfwWaitEventsTimeout(timeout);
    }
}
