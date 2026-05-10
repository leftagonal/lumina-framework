#pragma once

#include "stream.hpp"

#include <memory>

namespace lumina::events {
    class Multistream final {
        using StreamType = std::unique_ptr<GenericStream>;

    public:
        Multistream() = default;
        ~Multistream() = default;

        Multistream(const Multistream&) = delete;
        Multistream(Multistream&&) noexcept = default;

        Multistream& operator=(const Multistream&) = delete;
        Multistream& operator=(Multistream&&) noexcept = default;

        template <auto Fn>
        [[nodiscard]] StreamToken connect() {
            using EventType = extraction::EventType<Fn>;

            auto& stream = get<EventType>();

            return stream.template connect<Fn>();
        }

        template <auto Fn>
        [[nodiscard]] StreamToken connect(extraction::InstanceType<Fn>& instance) {
            using EventType = extraction::EventType<Fn>;

            auto& stream = get<EventType>();

            return stream.template connect<Fn>(instance);
        }

        void disconnect(const StreamToken& token) {
            if (token.typeIndex() >= streams_.size()) {
                return;
            }

            if (streams_[token.index()]) {
                streams_[token.index()]->disconnect(token);
            }
        }

        template <Event T>
        [[nodiscard]] Stream<T>& get() {
            std::size_t index = typeIndex<T>();

            if (index >= streams_.size()) {
                streams_.resize(std::max(4ul, index * 2));
            }

            auto& generic = streams_[index];

            if (!generic) {
                generic = std::make_unique<Stream<T>>();
            }

            return *static_cast<Stream<T>*>(generic.get());
        }

        template <Event T>
        void enqueue(const T& event) {
            auto& stream = get<T>();

            stream.enqueue(event);
        }

        template <Event T>
        void enqueue(T&& event) {
            auto& stream = get<T>();

            stream.enqueue(event);
        }

        template <Event T>
        void trigger(const T& event) {
            auto& stream = get<T>();

            stream.trigger(event);
        }

        template <Event T>
        void trigger(T&& event) {
            auto& stream = get<T>();

            stream.trigger(event);
        }

        void dispatch() {
            for (auto& stream : streams_) {
                if (stream) {
                    stream->dispatch();
                }
            }
        }

    private:
        std::vector<StreamType> streams_;
    };
}
