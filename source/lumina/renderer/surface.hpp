#pragma once

#include <lumina/system/window_manager.hpp>

namespace lumina::renderer {
    /// @brief A mode of presentation a surface can use.
    enum class SurfacePresentMode {
        /// @brief Waits for the next v-blank.
        /// Always supported.
        Synchronous,

        /// @brief Waits for the next v-blank, allowing late submissions.
        /// Falls back to Synchronous if unsupported.
        Semisynchonous,

        /// @brief Overwrites the oldest frame, presents the newest frame on v-blank.
        /// Falls back to Semisynchonous if unsupported.
        Asynchronous,

        /// @brief Immediately displays the frame.
        /// Falls back to Asynchronous if unsupported.
        Immediate,
    };

    enum class SurfaceFormat {

    };

    enum class SurfaceColourSpace {

    };

    class Instance;

    namespace accessors {
        class SurfaceAccessor;
    }

    class Surface final {
    public:
        Surface() = default;
        ~Surface();

        Surface(Instance& instance, system::WindowManager& windowManager, const system::WindowHandle& windowHandle);

        Surface(const Surface&) = delete;
        Surface(Surface&&) noexcept;

        Surface& operator=(const Surface&) = delete;
        Surface& operator=(Surface&&) noexcept;

        explicit operator bool() const;

        void create(Instance& instance, system::WindowManager& windowManager, const system::WindowHandle& windowHandle);
        void destroy();

        [[nodiscard]] bool valid() const;

        [[nodiscard]] Instance& instance();
        [[nodiscard]] const Instance& instance() const;

    private:
        Instance* instance_ = nullptr;

        VkSurfaceKHR surface_ = nullptr;

        friend class accessors::SurfaceAccessor;
    };

    namespace accessors {
        class SurfaceAccessor {
        public:
            SurfaceAccessor() = delete;

            [[nodiscard]] static VkSurfaceKHR& surface(Surface& surface) {
                return surface.surface_;
            }

            [[nodiscard]] static const VkSurfaceKHR& surface(const Surface& surface) {
                return surface.surface_;
            }
        };
    }
}
