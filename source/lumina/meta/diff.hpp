#pragma once

namespace lumina::meta {
    template <typename T>
    struct Diff {
        T previous;
        T current;
    };
}
