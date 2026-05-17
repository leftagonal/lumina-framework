#pragma once

#include <lumina/system/window_manager.hpp>

namespace lumina::renderer {
    /// @brief A mode of presentation a surface can use.
    enum class PresentMode {
        /// @brief Waits for the next v-blank.
        /// Always supported.
        VSync,

        /// @brief Waits for the next v-blank, allowing late submissions.
        VSyncRelaxed,

        /// @brief Overwrites the oldest frame, presents the newest frame on v-blank.
        Mailbox,

        /// @brief Immediately displays the frame.
        Immediate,
    };

    /// @brief The format an image stores data as.
    enum class Format {
        /// @brief 8-bit BGRA, hardware sRGB encoded.
        B8G8R8A8_sRGB,

        /// @brief 8-bit RGBA, hardware sRGB encoded.
        R8G8B8A8_sRGB,
    };

    /// @brief The colour space used when presenting images on a surface.
    enum class ColourSpace {
        /// @brief Nonlinear sRGB conversion.
        Nonlinear_sRGB,
    };

    namespace converters {
        class SurfaceEnumConverter {
        public:
            SurfaceEnumConverter() = delete;

            [[nodiscard]] static VkPresentModeKHR map(PresentMode presentMode) {
                switch (presentMode) {
                    case PresentMode::VSync: {
                        return VK_PRESENT_MODE_FIFO_KHR;
                    }
                    case PresentMode::VSyncRelaxed: {
                        return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
                    }
                    case PresentMode::Mailbox: {
                        return VK_PRESENT_MODE_MAILBOX_KHR;
                    }
                    case PresentMode::Immediate: {
                        return VK_PRESENT_MODE_IMMEDIATE_KHR;
                    }
                }
            }

            [[nodiscard]] static VkFormat map(Format imageFormat) {
                switch (imageFormat) {
                    case Format::R8G8B8A8_sRGB: {
                        return VK_FORMAT_R8G8B8A8_SRGB;
                    }
                    case Format::B8G8R8A8_sRGB: {
                        return VK_FORMAT_B8G8R8A8_SRGB;
                    }
                }
            }

            [[nodiscard]] static VkColorSpaceKHR map(ColourSpace colourSpace) {
                switch (colourSpace) {
                    case ColourSpace::Nonlinear_sRGB: {
                        return VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
                    }
                }
            }

            [[nodiscard]] static PresentMode map(VkPresentModeKHR presentMode) {
                switch (presentMode) {
                    case VK_PRESENT_MODE_FIFO_KHR: {
                        return PresentMode::VSync;
                    }
                    case VK_PRESENT_MODE_FIFO_RELAXED_KHR: {
                        return PresentMode::VSyncRelaxed;
                    }
                    case VK_PRESENT_MODE_MAILBOX_KHR: {
                        return PresentMode::Mailbox;
                    }
                    case VK_PRESENT_MODE_IMMEDIATE_KHR: {
                        return PresentMode::Immediate;
                    }
                    default: {
                        meta::fail("unsupported Vulkan surface present mode");
                    }
                }
            }

            [[nodiscard]] static Format map(VkFormat imageFormat) {
                switch (imageFormat) {
                    case VK_FORMAT_R8G8B8A8_SRGB: {
                        return Format::R8G8B8A8_sRGB;
                    }
                    case VK_FORMAT_B8G8R8A8_SRGB: {
                        return Format::B8G8R8A8_sRGB;
                    }
                    default: {
                        meta::fail("unsupported Vulkan surface format");
                    }
                }
            }

            [[nodiscard]] static ColourSpace map(VkColorSpaceKHR colourSpace) {
                switch (colourSpace) {
                    case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR: {
                        return ColourSpace::Nonlinear_sRGB;
                    }
                    default: {
                        meta::fail("unsupported Vulkan surface colour space");
                    }
                }
            }

            [[nodiscard]] static bool mappable(VkPresentModeKHR presentMode) {
                switch (presentMode) {
                    case VK_PRESENT_MODE_FIFO_KHR:
                    case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
                    case VK_PRESENT_MODE_MAILBOX_KHR:
                    case VK_PRESENT_MODE_IMMEDIATE_KHR: {
                        return true;
                    }
                    default: {
                        return false;
                    }
                }
            }

            [[nodiscard]] static bool mappable(VkFormat imageFormat) {
                switch (imageFormat) {
                    case VK_FORMAT_R8G8B8A8_SRGB:
                    case VK_FORMAT_B8G8R8A8_SRGB: {
                        return true;
                    }
                    default: {
                        return false;
                    }
                }
            }

            [[nodiscard]] static bool mappable(VkColorSpaceKHR colourSpace) {
                switch (colourSpace) {
                    case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR: {
                        return true;
                    }
                    default: {
                        return false;
                    }
                }
            }
        };
    }

    struct SurfaceFormat {
        Format imageFormat;
        ColourSpace colourSpace;
    };

    class Instance;
    class LogicalDevice;

    struct SurfaceCapabilities {
        std::uint32_t minimumImageCount;
        std::uint32_t maximumImageCount;

        meta::Extent2D<std::uint32_t> minimumExtent;
        meta::Extent2D<std::uint32_t> maximumExtent;
        meta::Extent2D<std::uint32_t> currentExtent;
    };

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
        [[nodiscard]] system::Window& window();
        [[nodiscard]] const system::Window& window() const;

        [[nodiscard]] std::vector<PresentMode> presentModes(const LogicalDevice& device) const;
        [[nodiscard]] std::vector<SurfaceFormat> formats(const LogicalDevice& device) const;
        [[nodiscard]] SurfaceCapabilities capabilities(const LogicalDevice& device) const;

    private:
        Instance* instance_ = nullptr;
        system::Window* window_ = nullptr;

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
