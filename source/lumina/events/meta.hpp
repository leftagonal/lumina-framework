#pragma once

#include <lumina/meta/type_index.hpp>
#include <lumina/meta/pod_type.hpp>

namespace lumina::events {
    template <typename T>
    concept Event = meta::PODType<T>;

    template <typename T>
    [[nodiscard]] std::size_t typeIndex() {
        return meta::typeIndex<meta::EventsTypeContext, T>();
    }
}
