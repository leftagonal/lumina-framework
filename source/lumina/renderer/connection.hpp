#pragma once

#include "display_server.hpp"
#include "logical_device.hpp"

namespace lumina::renderer {
    class Instance;

    /**
     * @brief Describes a Connection.
     *
     */
    struct ConnectionInfo {
        /// @brief The first window of the connection.
        WindowInfo pilotWindowInfo;

        /// @brief The requirements of the GPU to select.
        DeviceRequirements deviceRequirements;
    };

    struct OpenResult {
        WindowHandle pilotWindow;
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
         * @return OpenResult The result of opening the connection.
         */
        [[nodiscard]] OpenResult open(const ConnectionInfo& connectionInfo);

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
        [[nodiscard]] WindowHandle createWindow(const WindowInfo& windowInfo);

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
        LogicalDevice logicalDevice_;
        DisplayServer displayServer_;

        Instance* instance_;
    };
}
