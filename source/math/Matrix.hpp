#pragma once

#include <array>
#include <cassert>
#include <concepts>
#include <span>

namespace math {
    template <std::floating_point T, std::size_t Rows, std::size_t Cols>
    class Matrix {
      public:
        static constexpr auto size() noexcept { return Rows * Cols; }
        static constexpr auto rows() noexcept { return Rows; }
        static constexpr auto cols() noexcept { return Cols; }

        constexpr auto first() noexcept { return &elems[0]; };
        constexpr auto first() const noexcept { return &elems[0]; }

      public:
        Matrix() = default;

        Matrix(const Matrix<T, Rows, Cols> &other) = default;
        Matrix &operator=(const Matrix<T, Rows, Cols> &other) = default;

        Matrix(Matrix<T, Rows, Cols> &&other) noexcept = default;
        Matrix &operator=(Matrix<T, Rows, Cols> &&) noexcept = default;

        explicit Matrix(std::span<const T, Rows> views) noexcept { std::ranges::copy(views, first()); }
        explicit Matrix(T value) noexcept { std::ranges::fill(elems, value); }

        template <std::size_t N>
        explicit Matrix(const T (&initializer)[N]) noexcept {
            static_assert((N == cols() && rows() == 1) || (N == rows() && cols() == 1));
            std::ranges::copy_n(&initializer[0], size(), first());
        }

        explicit Matrix(const T (&initializer)[Rows][Cols]) noexcept { std::ranges::copy_n(&initializer[0][0], size(), first()); }

        constexpr auto operator[](std::size_t index) const {
            assert(index >= 0);

            if constexpr (cols() == 1) {
                assert(index < rows());
                return elems[index];
            } else if constexpr (rows() == 1) {
                assert(index < cols());
                return elems[index];
            } else {
                assert(index < rows());
                return std::span<T, Cols>{&elems[index * Cols], Cols};
            }
        }

      private:
        std::array<T, Rows * Cols> elems;
    };

    namespace detail {
        template <typename T>
        struct is_matrix : std::false_type {};

        template <typename T, std::size_t M, std::size_t N>
        struct is_matrix<Matrix<T, M, N>> : std::true_type {};

        template <typename T, std::size_t M, std::size_t N>
        struct is_matrix<const Matrix<T, M, N>> : std::true_type {};
    }  // namespace detail

    template <typename T>
    inline constexpr bool is_matrix_v = detail::is_matrix<T>::value;
}  // namespace math
