#pragma once

#include "Vector2.hpp"

#include <compare>

template <typename T>
	requires (std::is_integral_v<T> || std::is_floating_point_v<T>)
struct Rectangle {
	constexpr Rectangle() = default;
	explicit constexpr Rectangle(const Vector2<T> &position, const Vector2<std::type_identity_t<T>> &size) : position(position), size(size) {}

	friend constexpr Rectangle operator+(const Rectangle &r1, const Rectangle &r2) noexcept { return {r1.position + r2.position, r1.size + r2.size}; }
	friend constexpr Rectangle operator-(const Rectangle &r1, const Rectangle &r2) noexcept { return {r1.position - r2.position, r1.size - r2.size}; }

	constexpr bool operator<=>(const Rectangle &other) const noexcept = default;

	[[nodiscard]] constexpr auto &getPosition() const noexcept { return position; }
	[[nodiscard]] constexpr auto &getSize() const noexcept { return size; }

private:
	Vector2<T> position = {0, 0};
	Vector2<T> size = {0, 0};
};

