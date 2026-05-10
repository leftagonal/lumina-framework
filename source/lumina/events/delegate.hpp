#pragma once

#include "meta.hpp"

namespace lumina::events {
    namespace extraction {
        template <typename Fn>
        struct InfoOf;

        template <typename I, typename T>
        struct InfoOf<void (I::*)(const T&)> {
            using InstanceType = I;
            using EventType = T;
        };

        template <typename I, typename T>
        struct InfoOf<void (I::*)(const T&) const> {
            using InstanceType = I const;
            using EventType = T;
        };

        template <typename I, typename T>
        struct InfoOf<void (I::*)(const T&) volatile> {
            using InstanceType = I volatile;
            using EventType = T;
        };

        template <typename I, typename T>
        struct InfoOf<void (I::*)(const T&) const volatile> {
            using InstanceType = I const volatile;
            using EventType = T;
        };

        template <auto Fn>
        using InstanceType = InfoOf<decltype(Fn)>::InstanceType;

        template <auto Fn>
        using EventType = InfoOf<decltype(Fn)>::EventType;
    }

    template <Event T>
    class Delegate final {
        using InvokerType = void(const void*, const T&);

    public:
        using EventType = T;

        Delegate() = default;

        ~Delegate() {
            disconnect();
        }

        template <auto Fn>
        Delegate()
            : instance_(nullptr), invoker_(freeInvoker<Fn>) {
        }

        template <auto Fn, typename I>
        Delegate(I& instance)
            : instance_(&instance), invoker_(memberInvoker<Fn, I>) {
        }

        Delegate(const Delegate&) = default;
        Delegate(Delegate&&) noexcept = default;

        Delegate& operator=(const Delegate&) = default;
        Delegate& operator=(Delegate&&) noexcept = default;

        template <auto Fn>
        void connect() {
            instance_ = nullptr;
            invoker_ = freeInvoker<Fn>;
        }

        template <auto Fn>
        void connect(extraction::InstanceType<Fn>& instance) {
            instance_ = &instance;
            invoker_ = memberInvoker<Fn>;
        }

        void disconnect() {
            instance_ = nullptr;
            invoker_ = nullptr;
        }

        [[nodiscard]] bool valid() const noexcept {
            return invoker_ != nullptr;
        }

        explicit operator bool() const noexcept {
            return valid();
        }

        void invoke(const T& event) const {
            invoker_(instance_, event);
        }

        void operator()(const T& event) const {
            invoker_(instance_, event);
        }

    private:
        const void* instance_;
        InvokerType* invoker_ = nullptr;

        template <auto Fn>
        static void freeInvoker(const void*, const T& event) {
            Fn(event);
        }

        template <auto Fn>
        static void memberInvoker(const void* instance, const T& event) {
            using InstanceType = extraction::InstanceType<Fn>;
            constexpr bool IsConst = std::is_const_v<InstanceType>;
            using CorrectedVoid = std::conditional_t<IsConst, const void*, void*>;

            CorrectedVoid corrected = nullptr;

            if constexpr (IsConst) {
                corrected = instance;
            } else {
                corrected = const_cast<CorrectedVoid>(instance);
            }

            InstanceType* castInstance = static_cast<InstanceType*>(corrected);

            (castInstance->*Fn)(event);
        }
    };
}
