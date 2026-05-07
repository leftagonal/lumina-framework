#pragma once

#include <cstdint>
#include <string_view>

namespace lumina::framework {
    static constexpr std::string_view Name = "Lumina Framework";
    static constexpr std::uint32_t VersionMajor = LUMINA_VERSION_MAJOR;
    static constexpr std::uint32_t VersionMinor = LUMINA_VERSION_MINOR;
    static constexpr std::uint32_t VersionPatch = LUMINA_VERSION_PATCH;
}