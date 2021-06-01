#pragma once

#include <compare>
#include <type_traits>

template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
struct Vector2 final {
    constexpr Vector2() noexcept : x(0), y(0) {}
    constexpr Vector2(T _x, std::type_identity_t<T> _y) noexcept : x(_x), y(_y) {}

    constexpr Vector2(const Vector2 &other) noexcept = default;
    constexpr Vector2 &operator=(const Vector2 &other) noexcept = default;

    constexpr Vector2(Vector2 &&other) noexcept = default;
    constexpr Vector2 &operator=(Vector2 &&other) noexcept = default;

    constexpr bool operator<=>(const Vector2 &other) const noexcept = default;

    friend constexpr Vector2 operator+(const Vector2 &v1, const Vector2 &v2) noexcept { return {v1.x + v2.x, v1.y + v2.y}; }
    friend constexpr Vector2 operator-(const Vector2 &v1, const Vector2 &v2) noexcept { return {v1.x - v2.x, v1.y - v2.y}; }

    T x{};
    T y{};
};

using Vector2i = Vector2<int>;
using Vector2u = Vector2<uint32_t>;

using Vector2f = Vector2<float>;
