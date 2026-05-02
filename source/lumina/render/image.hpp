#pragma once

#include <lumina/render/device.hpp>

namespace lumina::render {
    struct ImageInfo {
        Device& device;
    };

    class Image {
    public:
        Image(const ImageInfo& info)
            : ownership_(true), device_(&info.device) {
        }

        Image(VkImage image)
            : ownership_(false), image_(image) {
        }

        ~Image() {
            if (image_ && ownership_) {
                vkDestroyImage(device_->handle(), image_, nullptr);
                image_ = nullptr;
                ownership_ = false;
            }
        }

    private:
        bool ownership_;
        VkImage image_;
        Device* device_;
    };
}