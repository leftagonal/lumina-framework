#pragma once

namespace lumina::system {
    class Instance final {
    public:
        Instance();
        ~Instance();

        Instance(const Instance&) = delete;
        Instance(Instance&&) noexcept = default;

        Instance& operator=(const Instance&) = delete;
        Instance& operator=(Instance&&) noexcept = default;

        void poll();
        void await();
        void await(double timeout);
    };
}
