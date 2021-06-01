#pragma once

#include <compare>
#include <type_traits>

template <typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
struct Vector3 final {
    constexpr Vector3() noexcept : x(0), y(0) {}
    constexpr Vector3(T _x, std::type_identity_t<T> _y, std::type_identity_t<T> _z) noexcept : x(_x), y(_y), z(_z) {}

    constexpr Vector3(const Vector3 &other) noexcept = default;
    constexpr Vector3 &operator=(const Vector3 &other) noexcept = default;

    constexpr Vector3(Vector3 &&other) noexcept = default;
    constexpr Vector3 &operator=(Vector3 &&other) noexcept = default;

    constexpr bool operator<=>(const Vector3 &other) const noexcept = default;

    friend constexpr Vector3 operator+(const Vector3 &v1, const Vector3 &v2) noexcept { return {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z}; }
    friend constexpr Vector3 operator-(const Vector3 &v1, const Vector3 &v2) noexcept { return {v1.x - v2.x, v1.y - v2.y, v1.z - v2.z}; }

    T x{};
    T y{};
    T z{};
};

using Vector3i = Vector3<int>;
using Vector3u = Vector3<uint32_t>;

using Vector3f = Vector3<float>;
