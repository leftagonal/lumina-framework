#pragma once

#include <cstdint>
#include <string_view>

#ifndef LUMINA_VERSION_MAJOR
#error lumina framework version has not been fully declared (missing: major)
#endif

#ifndef LUMINA_VERSION_MINOR
#error lumina framework version has not been fully declared (missing: minor)
#endif

#ifndef LUMINA_VERSION_PATCH
#error lumina framework version has not been fully declared (missing: patch)
#endif

namespace lumina::framework {
    static constexpr std::string_view Name = "Lumina Framework";
    static constexpr std::uint32_t VersionMajor = LUMINA_VERSION_MAJOR;
    static constexpr std::uint32_t VersionMinor = LUMINA_VERSION_MINOR;
    static constexpr std::uint32_t VersionPatch = LUMINA_VERSION_PATCH;
}