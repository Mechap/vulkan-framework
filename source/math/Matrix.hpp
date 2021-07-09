#pragma once

#include <array>
#include <concepts>
#include <span>

namespace math {
    template <typename T, std::regular R, std::size_t N>
    class Matrix {
      public:
        static constexpr std::size_t ndim = N;

        using value_type = R;
        using reference = std::remove_reference_t<R> &;
        using const_reference = std::remove_reference_t<R> &;
        using pointer = R *;

      public:
        Matrix() = delete;
        virtual ~Matrix() = default;

        constexpr Matrix(std::size_t _size, std::span<const std::size_t, N> _dims, std::span<const std::size_t, N> _strides) : size(_size), dims(_dims), strides(_strides) {}

        friend void swap(Matrix &a, Matrix &b) noexcept {
            std::swap(a.size, b.size);
            std::swap(a.dims, b.dims);
            std::swap(a.strides, b.strides);
        }

        auto begin() { return static_cast<T &>(*this); }
        auto cbegin() const { return static_cast<const T &>(*this); }

      private:
        std::size_t size;

        std::array<std::size_t, N> dims;
        std::array<std::size_t, N> strides;
    };
}  // namespace math
