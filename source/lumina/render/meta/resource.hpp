#pragma once

#include <type_traits>

namespace lumina::render {
    /**
     * @brief An owning resource wrapper.
     *
     * @tparam T The type of the resource.
     */
    template <typename T>
    requires(std::is_pointer_v<T>)
    class Resource {
    public:
        Resource() = default;

        /**
         * @brief Construct a new resource.
         *
         * @param resource The resource itself.
         * @param ownership Whether this now owns the resource. (default is referencing)
         */
        Resource(T resource)
            : resource_(resource) {
        }

        Resource(const Resource&) = delete;

        /**
         * @brief Move-construct a new resource.
         *
         * @param other The other resource. (source)
         */
        Resource(Resource&& other) noexcept
            : resource_(other.resource_) {
            other.resource_ = nullptr;
        }

        Resource& operator=(const Resource&) = delete;

        /**
         * @brief Move-assign a new resource.
         *
         * @param other The other resource. (source)
         * @return Resource& The newly created resource.
         */
        Resource& operator=(Resource&& other) noexcept {
            if (this == &other) {
                return *this;
            }

            resource_ = other.resource_;

            other.resource_ = nullptr;

            return *this;
        }

        /**
         * @brief Compare two resources for equality.
         *
         * @param other The right-hand operand resource.
         * @return true If the resources are identical.
         * @return false If the resources differ.
         */
        [[nodiscard]] bool operator==(const Resource& other) const noexcept {
            return resource_ == other.resource_;
        }

        /**
         * @brief Compare two resources for inequality.
         *
         * @param other The right-hand operand resource.
         * @return true If the resources differ.
         * @return false If the resources are identical.
         */
        [[nodiscard]] bool operator!=(const Resource& other) const noexcept {
            return resource_ != other.resource_;
        }

        /**
         * @brief Provide the resource itself.
         *
         * @return T The resource.
         */
        [[nodiscard]] operator T() const noexcept {
            return resource_;
        }

        /**
         * @brief Provide whether the resource is in a valid state.
         *
         * @return true If the resource is valid.
         * @return false If the resource is invalid.
         */
        [[nodiscard]] explicit operator bool() const noexcept {
            return resource_ != nullptr;
        }

        /**
         * @brief Provide whether the resource is in a valid state.
         *
         * @return true If the resource is valid.
         * @return false If the resource is invalid.
         */
        [[nodiscard]] bool valid() const noexcept {
            return resource_ != nullptr;
        }

        /**
         * @brief Provide the resource itself.
         *
         * @return T The resource.
         */
        [[nodiscard]] T get() const noexcept {
            return resource_;
        }

    protected:
        /**
         * @brief Provides access to the resource directly.
         *
         * @return T& A reference to the resource.
         */
        [[nodiscard]] T& resource() noexcept {
            return resource_;
        }

        /**
         * @brief Provides access to the resource directly.
         *
         * @return const T& A reference to the resource.
         */
        [[nodiscard]] const T& resource() const noexcept {
            return resource_;
        }

        /**
         * @brief Invalidates the resource.
         *
         * Subsequent valid() checks or bool conversions will yield false.
         */
        void invalidate() {
            resource_ = nullptr;
        }

    private:
        T resource_ = nullptr;
    };
}