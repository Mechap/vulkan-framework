#pragma once

#include <deque>
#include <functional>
#include <ranges>

struct NoCopy {
    NoCopy() = default;

    NoCopy(const NoCopy &) = delete;
    NoCopy &operator=(const NoCopy &) = delete;
};

struct NoMove {
    NoMove() = default;

    NoMove(NoMove &&) = delete;
    NoMove &operator=(NoMove &&) = delete;
};

struct DeletionQueue final {
	inline static std::vector<std::function<void()>> deletors;

	static void push_function(std::function<void()> &&function) noexcept {
		deletors.push_back(std::move(function));
	}

	static void flush() noexcept {
		for (auto it : deletors | std::views::reverse) {
			(it)();
		}

		deletors.clear();
	}
};
