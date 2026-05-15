#pragma once

#include "stream.hpp"

#include <memory>

namespace lumina::events {
    template <Event T>
    class Socket;

    class Multistream final {
        template <Event T>
        friend class lumina::events::Socket;

        using StreamType = std::unique_ptr<GenericStream>;

    public:
        Multistream() = default;
        ~Multistream() = default;

        Multistream(const Multistream&) = delete;
        Multistream(Multistream&&) noexcept = default;

        Multistream& operator=(const Multistream&) = delete;
        Multistream& operator=(Multistream&&) noexcept = default;

        void dispatch() {
            for (auto& stream : streams_) {
                if (stream) {
                    stream->dispatch();
                }
            }
        }

    private:
        std::vector<StreamType> streams_;

        template <Event T>
        void ensure() {
            std::size_t index = typeIndex<T>();

            if (streams_.size() <= index) {
                streams_.resize(index + 1);
            }

            auto& generic = streams_[index];

            if (!generic) {
                generic = std::make_unique<Stream<T>>();
            }
        }
    };
}
