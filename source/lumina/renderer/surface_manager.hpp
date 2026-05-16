#pragma once

#include <lumina/system/window_manager.hpp>

namespace lumina::renderer {
    class Instance;

    namespace accessors {
        class SurfaceManagerAccessor;
    }

    class SurfaceManager final {
    public:
        SurfaceManager() = default;
        ~SurfaceManager();

        SurfaceManager(Instance& instance, system::WindowManager& windowManager);

        SurfaceManager(const SurfaceManager&) = delete;
        SurfaceManager(SurfaceManager&&) noexcept;

        SurfaceManager& operator=(const SurfaceManager&) = delete;
        SurfaceManager& operator=(SurfaceManager&&) noexcept;

        explicit operator bool() const;

        void create(Instance& instance, system::WindowManager& windowManager);
        void destroy();

        [[nodiscard]] bool valid() const;
        [[nodiscard]] bool empty() const;

    private:
        Instance* instance_ = nullptr;
        system::WindowManager* windowManager_ = nullptr;

        events::Token createToken_;
        events::Token destroyToken_;

        std::vector<VkSurfaceKHR> surfaces_;

        void createAll();
        void destroyAll();

        void onWindowCreate(const system::WindowManagerWindowCreateEvent& event);
        void onWindowDestroy(const system::WindowManagerWindowDestroyEvent& event);

        friend class accessors::SurfaceManagerAccessor;
    };

    namespace accessors {
        class SurfaceManagerAccessor {
        public:
            SurfaceManagerAccessor() = delete;

            [[nodiscard]] static VkSurfaceKHR firstValidSurface(const SurfaceManager& surfaceManager) {
                for (auto& surface : surfaceManager.surfaces_) {
                    if (surface != nullptr) {
                        return surface;
                    }
                }

                return nullptr;
            }

            [[nodiscard]] static VkSurfaceKHR& surface(SurfaceManager& surfaceManager, system::WindowHandle handle) {
                return surfaceManager.surfaces_[handle.id()];
            }

            [[nodiscard]] static const VkSurfaceKHR& surface(const SurfaceManager& surfaceManager, system::WindowHandle handle) {
                return surfaceManager.surfaces_[handle.id()];
            }
        };
    }
}
