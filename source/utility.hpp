#pragma once

#include <deque>
#include <functional>

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
	inline static std::deque<std::function<void()>> deletors;

	static void push_function(std::function<void()> &&function) noexcept {
		deletors.push_back(std::move(function));
	}

	static void flush() noexcept {
		for (auto it = deletors.rbegin(); it != deletors.rend(); ++it) {
			(*it)();
		}
		deletors.clear();
	}
};
