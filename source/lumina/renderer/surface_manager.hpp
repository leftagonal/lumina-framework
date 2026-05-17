#pragma once

#include "surface.hpp"

namespace lumina::renderer {
    class Instance;

    struct SurfaceManagerSurfaceDestroyEvent {
        system::WindowHandle target;
    };

    using SurfaceManagerSurfaceDestroySocket = events::Socket<SurfaceManagerSurfaceDestroyEvent>;

    struct SurfaceManagerEventSockets {
        SurfaceManagerSurfaceDestroySocket destroySocket;
    };

    namespace accessors {
        class SurfaceManagerAccessor;
    }

    class SurfaceManager final {
    public:
        SurfaceManager() = default;
        ~SurfaceManager();

        SurfaceManager(Instance& instance, system::WindowManager& windowManager, const SurfaceManagerEventSockets& sockets);

        SurfaceManager(const SurfaceManager&) = delete;
        SurfaceManager(SurfaceManager&&) noexcept;

        SurfaceManager& operator=(const SurfaceManager&) = delete;
        SurfaceManager& operator=(SurfaceManager&&) noexcept;

        explicit operator bool() const;

        void create(Instance& instance, system::WindowManager& windowManager, const SurfaceManagerEventSockets& sockets);
        void destroy();

        [[nodiscard]] bool valid() const;
        [[nodiscard]] bool empty() const;

        [[nodiscard]] std::vector<Surface>& surfaces();
        [[nodiscard]] const std::vector<Surface>& surfaces() const;

        [[nodiscard]] Surface& get(const system::WindowHandle& windowHandle);
        [[nodiscard]] const Surface& get(const system::WindowHandle& windowHandle) const;
        [[nodiscard]] bool contains(const system::WindowHandle& windowHandle) const;

        [[nodiscard]] SurfaceManagerEventSockets sockets() const;

    private:
        Instance* instance_ = nullptr;
        system::WindowManager* windowManager_ = nullptr;

        events::Token createToken_;
        events::Token destroyToken_;
        std::vector<Surface> surfaces_;
        SurfaceManagerEventSockets sockets_;

        void createAll();

        void onWindowCreate(const system::WindowManagerWindowCreateEvent& event);
        void onWindowDestroy(const system::WindowManagerWindowDestroyEvent& event);

        friend class accessors::SurfaceManagerAccessor;
    };
}
