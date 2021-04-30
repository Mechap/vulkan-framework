#pragma once

#include <type_traits>
#include <concepts>

template <typename T>
	requires (std::is_integral_v<T> || std::is_floating_point_v<T>)
struct Vector2 final {
    constexpr Vector2() noexcept : x(0), y(0) {}
    constexpr Vector2(T _x, std::type_identity_t<T> _y) noexcept : x(_x), y(_y) {}

    friend constexpr Vector2 operator+(const Vector2 &v1, const Vector2 &v2) noexcept { return {v1.x + v2.x, v1.y + v2.y}; }
    friend constexpr Vector2 operator-(const Vector2 &v1, const Vector2 &v2) noexcept { return {v1.x - v2.x, v1.y - v2.y}; }

    T x{};
    T y{};
};

using Vector2i = Vector2<int>;
using Vector2u = Vector2<uint32_t>;

using Vector2f = Vector2<float>;
