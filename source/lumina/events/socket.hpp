#pragma once

#include <lumina/meta/exceptions.hpp>

#include "meta.hpp"
#include "multistream.hpp"
#include "stream.hpp"

namespace lumina::events {
    enum class SocketMode {
        Solitary,
        Indexed,
    };

    template <Event T>
    class Socket final {
        using StreamType = std::unique_ptr<GenericStream>;

    public:
        using EventType = T;
        using DelegateType = Delegate<T>;

        Socket() = default;
        ~Socket() = default;

        Socket(Multistream& multistream) {
            multistream.ensure<T>();

            index_ = typeIndex<T>();
            storage_.streams = &multistream.streams_;
        }

        Socket(Stream<T>& stream)
            : index_(Sentinel) {
            storage_.stream = &stream;
        }

        Socket(const Socket&) = default;

        Socket(Socket&& other) noexcept
            : index_(other.index_), storage_(other.storage_) {
            other.index_ = Sentinel;
            other.storage_ = {};
        }

        Socket& operator=(const Socket&) = default;

        Socket& operator=(Socket&& other) noexcept {
            if (this == &other) {
                return *this;
            }

            index_ = other.index_;
            storage_ = other.storage_;

            other.index_ = Sentinel;
            other.storage_ = {};

            return *this;
        }

        [[nodiscard]] bool valid() const {
            if (index_ == Sentinel) {
                return storage_.stream != nullptr;
            }

            return storage_.streams != nullptr && storage_.streams->size() > index_;
        }

        explicit operator bool() const {
            return valid();
        }

        Stream<T>* operator->() {
            return &get();
        }

        const Stream<T>* operator->() const {
            return &get();
        }

        [[nodiscard]] SocketMode mode() const {
            if (index_ == Sentinel) {
                return SocketMode::Solitary;
            }

            return SocketMode::Indexed;
        }

        [[nodiscard]] Stream<T>& get() {
            if (index_ == Sentinel) {
                return *storage_.stream;
            }

            auto& stream = (*storage_.streams)[index_];

            return static_cast<Stream<T>&>(*stream.get());
        }

        [[nodiscard]] const Stream<T>& get() const {
            if (index_ == Sentinel) {
                return *storage_.stream;
            }

            auto& stream = (*storage_.streams)[index_];

            return static_cast<const Stream<T>&>(*stream.get());
        }

        void connect(Multistream& multistream) {
            multistream.ensure<T>();

            index_ = typeIndex<T>();
            storage_.streams = &multistream.streams_;
        }

        void connect(Stream<T>& stream) {
            index_ = Sentinel;
            storage_.stream = &stream;
        }

        void disconnect() {
            index_ = Sentinel;
            storage_ = {};
        }

    private:
        union Storage {
            std::vector<StreamType>* streams = nullptr;
            Stream<T>* stream;
        };

        std::size_t index_ = Sentinel;
        Storage storage_ = {};

        static constexpr std::size_t Sentinel = SIZE_T_MAX;
    };
}
