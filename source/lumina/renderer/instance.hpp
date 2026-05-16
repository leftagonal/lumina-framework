#pragma once

#include <lumina/core/application.hpp>
#include <lumina/core/vulkan.hpp>

#include <vector>

namespace lumina::renderer {
    namespace accessors {
        struct InstanceAccessor;
    }

    class Instance {
    public:
        Instance() = default;
        ~Instance();

        Instance(const core::ApplicationInfo& info);

        Instance(const Instance&) = delete;
        Instance(Instance&&) noexcept;

        Instance& operator=(const Instance&) = delete;
        Instance& operator=(Instance&&) noexcept;

        explicit operator bool() const;

        void create(const core::ApplicationInfo& info);
        void destroy();

        [[nodiscard]] const core::ApplicationInfo& applicationInfo() const;

        [[nodiscard]] bool valid() const;

    private:
        using ExtensionProperties = std::vector<VkExtensionProperties>;
        using LayerProperties = std::vector<VkLayerProperties>;
        using Names = std::vector<const char*>;

        core::ApplicationInfo applicationInfo_;
        VkInstance instance_ = nullptr;

        [[nodiscard]] bool validation() const;

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
