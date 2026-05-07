#pragma once

#include "vulkan.hpp"

#include <cstdint>
#include <string_view>
#include <vector>

namespace lumina::renderer {
    struct Version {
        std::uint32_t major;
        std::uint32_t minor;
        std::uint32_t patch;
    };

    struct InstanceFeatures {
        bool validation;
        bool debugging;
    };

    struct InstanceInfo {
        std::string_view name;
        Version version;
        InstanceFeatures features;
    };

    namespace accessors {
        struct InstanceAccessor;
    }

    class Instance {
    public:
        Instance(const InstanceInfo& info);
        ~Instance();

        Instance(const Instance&) = delete;
        Instance(Instance&&) noexcept = default;

        Instance& operator=(const Instance&) = delete;
        Instance& operator=(Instance&&) noexcept = default;

        [[nodiscard]] bool debugging() const {
            return debugging_;
        }

    private:
        using ExtensionProperties = std::vector<VkExtensionProperties>;
        using LayerProperties = std::vector<VkLayerProperties>;
        using Names = std::vector<const char*>;

        VkInstance instance_ = nullptr;
        bool debugging_;

        void initialiseSystemAPI() const;
        void appendRequiredExtensions(const InstanceFeatures& features, Names& requirements) const;
        void appendRequiredLayers(const InstanceFeatures& features, Names& requirements) const;

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