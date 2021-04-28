#pragma once

#include <type_traits>

template <typename T>
struct Vector2 final {
    constexpr Vector2() noexcept : x(0), y(0) {}
    constexpr Vector2(T _x, T _y) noexcept : x(_x), y(_y) {}

    constexpr Vector2 operator+(const Vector2 &other) noexcept { return {x + other.x, y + other.y}; }
    constexpr Vector2 operator-(const Vector2 &other) noexcept { return {x - other.x, y - other.y}; }

    T x{};
    T y{};
};

using Vector2i = Vector2<int>;
using Vector2u = Vector2<uint32_t>;

using Vector2f = Vector2<float>;
