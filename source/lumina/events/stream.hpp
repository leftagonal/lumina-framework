#pragma once

#include "delegate.hpp"

#include <cstdlib>
#include <vector>

namespace lumina::events {
    class Token final {
    public:
        Token() = default;

        Token(std::size_t index, std::size_t typeIndex)
            : index_(index), typeIndex_(typeIndex) {
        }

        ~Token() = default;

        Token(const Token&) = delete;
        Token(Token&&) noexcept = default;

        Token& operator=(const Token&) = delete;
        Token& operator=(Token&&) noexcept = default;

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

        [[nodiscard]] virtual bool connected(const Token& token) const = 0;

        virtual void disconnect(const Token& token) = 0;
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
        Token connect() {
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
        Token connect(extraction::InstanceType<Fn>& instance) {
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

        [[nodiscard]] bool connected(const Token& token) const override {
            return token.index() < delegates_.size() && typeIndex<T>() == token.typeIndex() && delegates_[token.index()];
        }

        void disconnect(const Token& token) override {
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

        /**
         * @brief Triggers an event if someone is listening, enqueues if not.
         *
         * @param event The event to attempt.
         */
        void attempt(const T& event) {
            bool noDelegates = delegates_.size() - freeList_.size() == 0;

            if (noDelegates) {
                enqueue(event);
            } else {
                trigger(event);
            }
        }

        /**
         * @brief Triggers an event if someone is listening, enqueues if not.
         *
         * @param event The event to attempt.
         */
        void attempt(T&& event) {
            bool noDelegates = delegates_.size() - freeList_.size() == 0;

            if (noDelegates) {
                enqueue(event);
            } else {
                trigger(event);
            }
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
