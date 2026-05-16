#pragma once

#include "surface.hpp"

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

        [[nodiscard]] std::vector<Surface>& surfaces();
        [[nodiscard]] const std::vector<Surface>& surfaces() const;

    private:
        Instance* instance_ = nullptr;
        system::WindowManager* windowManager_ = nullptr;

        events::Token createToken_;
        events::Token destroyToken_;

        std::vector<Surface> surfaces_;

        void createAll();
        void destroyAll();

        void onWindowCreate(const system::WindowManagerWindowCreateEvent& event);
        void onWindowDestroy(const system::WindowManagerWindowDestroyEvent& event);

        friend class accessors::SurfaceManagerAccessor;
    };
}
