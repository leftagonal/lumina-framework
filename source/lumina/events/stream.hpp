#pragma once

#include "delegate.hpp"

#include <cstdlib>
#include <vector>

namespace lumina::events {
    class StreamToken final {
    public:
        StreamToken(std::size_t index, std::size_t typeIndex)
            : index_(index), typeIndex_(typeIndex) {
        }

        ~StreamToken() = default;

        StreamToken(const StreamToken&) = delete;
        StreamToken(StreamToken&&) noexcept = default;

        StreamToken& operator=(const StreamToken&) = delete;
        StreamToken& operator=(StreamToken&&) noexcept = default;

        [[nodiscard]] std::size_t index() const noexcept {
            return index_;
        }

        [[nodiscard]] std::size_t typeIndex() const noexcept {
            return typeIndex_;
        }

    private:
        std::size_t index_;
        std::size_t typeIndex_;
    };

    class GenericStream {
    public:
        GenericStream() = default;
        ~GenericStream() = default;

        GenericStream(const GenericStream&) = delete;
        GenericStream(GenericStream&&) noexcept = default;

        GenericStream& operator=(const GenericStream&) = delete;
        GenericStream& operator=(GenericStream&&) noexcept = default;

        [[nodiscard]] virtual bool connected(const StreamToken& token) const = 0;

        virtual void disconnect(const StreamToken& token) = 0;
        virtual void dispatch() = 0;
    };

    template <Event T>
    class Stream final : public GenericStream {
    public:
        using EventType = T;
        using DelegateType = Delegate<T>;

        Stream() = default;
        ~Stream() = default;

        Stream(const Stream&) = delete;
        Stream(Stream&&) noexcept = default;

        Stream& operator=(const Stream&) = delete;
        Stream& operator=(Stream&&) noexcept = default;

        template <auto Fn>
        [[nodiscard]] StreamToken connect() {
            std::size_t index = 0;

            if (!freeList_.empty()) {
                index = freeList_.back();
                freeList_.pop_back();
            } else {
                index = delegates_.size();
                delegates_.emplace_back();
            }

            delegates_[index].template connect<Fn>();

            return {index, typeIndex<T>()};
        }

        template <auto Fn>
        [[nodiscard]] StreamToken connect(extraction::InstanceType<Fn>& instance) {
            std::size_t index = 0;

            if (!freeList_.empty()) {
                index = freeList_.back();
                freeList_.pop_back();
            } else {
                index = delegates_.size();
                delegates_.emplace_back();
            }

            delegates_[index].template connect<Fn>(instance);

            return {index, typeIndex<T>()};
        }

        [[nodiscard]] bool connected(const StreamToken& token) const override {
            return token.index() < delegates_.size() && typeIndex<T>() == token.typeIndex() && delegates_[token.index()];
        }

        void disconnect(const StreamToken& token) override {
            if (!connected(token)) {
                return;
            }

            delegates_[token.index()].disconnect();
            freeList_.emplace_back(token.index());
        }

        void enqueue(const T& event) {
            eventQueue_.emplace_back(event);
        }

        void enqueue(T&& event) {
            eventQueue_.emplace_back(event);
        }

        void trigger(const T& event) const {
            for (auto& delegate : delegates_) {
                if (delegate) {
                    delegate(event);
                }
            }
        }

        void trigger(T&& event) const {
            for (auto& delegate : delegates_) {
                if (delegate) {
                    delegate(event);
                }
            }
        }

        void dispatch() override {
            for (auto& event : eventQueue_) {
                trigger(event);
            }

            eventQueue_.clear();
        }

        void disconnectAll() {
            delegates_.clear();
        }

        void clearEvents() {
            eventQueue_.clear();
        }

        [[nodiscard]] std::vector<DelegateType>& delegates() noexcept {
            return delegates_;
        }

        [[nodiscard]] const std::vector<DelegateType>& delegates() const noexcept {
            return delegates_;
        }

        [[nodiscard]] std::vector<EventType>& queue() noexcept {
            return eventQueue_;
        }

        [[nodiscard]] const std::vector<EventType>& queue() const noexcept {
            return eventQueue_;
        }

    private:
        std::vector<DelegateType> delegates_;
        std::vector<EventType> eventQueue_;
        std::vector<std::size_t> freeList_;
    };
}
