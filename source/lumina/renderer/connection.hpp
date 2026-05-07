#pragma once

#include "subsystems/device.hpp"
#include "subsystems/presenter.hpp"

namespace lumina::renderer {
    class Instance;

    /**
     * @brief Describes a Connection.
     *
     */
    struct ConnectionInfo {
        /// @brief The first window of the connection.
        WindowInfo initialWindow;

        /// @brief The requirements of the GPU to select.
        DeviceRequirements deviceRequirements;
    };

    /**
     * @brief Represents the connection between the application, the display server, and the selected GPU.
     *
     */
    class Connection {
    public:
        Connection(Instance& instance);
        ~Connection() = default;

        /**
         * @brief Connect to a GPU and display server.
         *
         * @param connectionInfo The description of the connection.
         * @return WindowHandle The first window of the connection.
         */
        WindowHandle open(const ConnectionInfo& connectionInfo);

        /**
         * @brief Disconnects from the GPU and display server.
         *
         */
        void close();

        /**
         * @brief Creates an additional window for use as a rendering target.
         *
         * @param windowInfo The setup information for the window.
         * @return WindowHandle The handle for the new window.
         */
        WindowHandle createWindow(const WindowInfo& windowInfo);

        /**
         * @brief Destroys a given window.
         *
         * @param handle The handle to the window to destroy.
         */
        void destroyWindow(WindowHandle handle);

        /**
         * @brief Retrieves the attributes of a given window.
         *
         * @param handle The handle to the window.
         * @return WindowAttributes The attributes of the window.
         */
        [[nodiscard]] WindowAttributes windowAttributes(WindowHandle handle) const;

    private:
        subsystems::Device device_;
        subsystems::Presenter presenter_;

        Instance* instance_;
    };
}