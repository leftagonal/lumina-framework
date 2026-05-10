#pragma once

#include <cstdint>
#include <string_view>

namespace lumina::core {
    struct Version {
        std::uint32_t major;
        std::uint32_t minor;
        std::uint32_t patch;
    };

    struct ApplicationFeatures {
        bool validation;
    };

    struct ApplicationInfo {
        std::string_view name;
        Version version;
        ApplicationFeatures features;
    };
}