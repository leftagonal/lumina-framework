#pragma once

#include <lumina/core/application.hpp>

#include "vulkan.hpp"

#include <cstdint>
#include <string_view>
#include <vector>

namespace lumina::renderer {
    namespace accessors {
        struct InstanceAccessor;
    }

    class Instance {
    public:
        Instance(const core::ApplicationInfo& info);
        ~Instance();

        Instance(const Instance&) = delete;
        Instance(Instance&&) noexcept = default;

        Instance& operator=(const Instance&) = delete;
        Instance& operator=(Instance&&) noexcept = default;

        [[nodiscard]] bool validation() const {
            return validation_;
        }

        void update();

    private:
        using ExtensionProperties = std::vector<VkExtensionProperties>;
        using LayerProperties = std::vector<VkLayerProperties>;
        using Names = std::vector<const char*>;

        VkInstance instance_ = nullptr;
        bool validation_;

        void initialiseSystemAPI() const;
        void appendRequiredExtensions(Names& requirements) const;
        void appendRequiredLayers(Names& requirements) const;

        [[nodiscard]] ExtensionProperties getAvailableExtensions() const;
        [[nodiscard]] LayerProperties getAvailableLayers() const;
        [[nodiscard]] VkInstanceCreateFlags enableCompatibility(Names& requirements, const ExtensionProperties& available) const;
        [[nodiscard]] bool testRequirements(const Names& required, const ExtensionProperties& available) const;
        [[nodiscard]] bool testRequirements(const Names& required, const LayerProperties& available) const;
        [[nodiscard]] std::uint32_t getDriverSupportedVersion() const;

        friend struct accessors::InstanceAccessor;
    };

    namespace accessors {
        struct InstanceAccessor {
            [[nodiscard]] static const VkInstance& instance(const Instance& instance) {
                return instance.instance_;
            }

            [[nodiscard]] static VkInstance& instance(Instance& instance) {
                return instance.instance_;
            }
        };
    }
}