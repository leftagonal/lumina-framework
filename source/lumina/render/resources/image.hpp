#pragma once

#include "../meta/resource.hpp"
#include "../systems/device.hpp"

namespace lumina::render {
    /**
     * @brief The providing party of an image.
     *
     * A regular provider means the image object is owning, but a swapchain
     * provider means the image object is non-owning, as the swapchain will
     * cleanup resources.
     */
    enum class ImageProvider {
        Regular,
        Swapchain,
    };

    /**
     * @brief An image in memory.
     *
     */
    class Image : public Resource<VkImage> {
    public:
        Image() = default;

        /**
         * @brief Construct a new Image object.
         *
         * @param device The device to create the image for.
         */
        Image(Device& device)
            : device_(&device) {
        }

        /**
         * @brief Construct a new Image object.
         *
         * @param image The VkImage to reference.
         * @param provider The provider of the image handle.
         *
         * The provider of the image dictates internal ownership rules.
         */
        Image(VkImage image, ImageProvider provider)
            : Resource(image), provider_(provider) {
        }

        /**
         * @brief Destroy the image.
         *
         * Will only destruct the VkImage if it is not owned by a swapchain.
         */
        ~Image() {
            destroy();
        }

        Image(Image&&) noexcept = default;

        Image& operator=(Image&&) noexcept = default;

        /**
         * @brief Construct the image.
         *
         * @param device The device to create the image for.
         */
        void create(Device& device) {
            if (*this) {
                return;
            }

            device_ = &device;
        }

        /**
         * @brief Destroy the image.
         *
         * Will only destruct the VkImage if it is not owned by a swapchain.
         */
        void destroy() {
            if (!*this) {
                return;
            }

            if (provider_ == ImageProvider::Regular) {
                vkDestroyImage(*device_, *this, nullptr);
            }

            device_ = nullptr;
            provider_ = ImageProvider::Regular;

            invalidate();
        }

        /**
         * @brief Provide the device that this image is for.
         *
         * @return Device& The backing logical device.
         */
        [[nodiscard]] Device& device() noexcept {
            return *device_;
        }

        /**
         * @brief Provide the device that this image is for.
         *
         * @return const Device& The backing logical device.
         */
        [[nodiscard]] const Device& device() const noexcept {
            return *device_;
        }

    private:
        ImageProvider provider_ = ImageProvider::Regular;

        Device* device_;
    };
}