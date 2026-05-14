#pragma once

#include <lumina/meta/type_index.hpp>
#include <lumina/meta/pod_type.hpp>

#include <vector>

namespace lumina::ecs {
    template <typename T>
    concept Component = meta::PODType<T>;

    template <typename T>
    [[nodiscard]] std::size_t typeIndex() {
        return meta::TypeIndexer<meta::ECSTypeContext>::index<T>();
    }

    using Versions = std::vector<std::size_t>;
}
