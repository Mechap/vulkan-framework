#pragma once

#include <fmt/core.h>

#include <algorithm>
#include <concepts>
#include <deque>
#include <functional>
#include <vector>

struct NoCopy {
    NoCopy() = default;

    NoCopy(const NoCopy &) = delete;
    NoCopy &operator=(const NoCopy &) = delete;

    NoCopy(NoCopy &&) noexcept = default;
    NoCopy &operator=(NoCopy &&) noexcept = default;
};

struct NoMove {
    NoMove() = default;

    NoMove(NoMove &&) = delete;
    NoMove &operator=(NoMove &&) = delete;

    NoMove(const NoMove &) noexcept = default;
    NoMove &operator=(const NoMove &) = default;
};

struct DeletionQueue final {
    inline static std::deque<std::function<void()>> deletors;

    static void push_function(std::function<void()> &&function) { deletors.push_front(std::move(function)); }

    static void flush() {
        for (const auto &it : deletors) {
            std::invoke(it);
        }

        deletors.clear();
    }
};

namespace nostd {
    template <typename T>
    class observer_ptr {
      public:
        using element_type = T;

        constexpr observer_ptr() noexcept = default;
        constexpr observer_ptr(std::nullptr_t) noexcept {};
        constexpr observer_ptr(element_type *ptr) noexcept : data(ptr) {}

        template <typename U>
			requires std::same_as<U, element_type> || std::convertible_to<U *, element_type *> 
		observer_ptr(const observer_ptr<U> &other)
            : observer_ptr(static_cast<element_type *>(other.get())) {}

        [[nodiscard]] constexpr element_type *get() const noexcept { return data; }
        [[nodiscard]] constexpr std::remove_reference_t<T> &operator*() const { return *get(); }
        [[nodiscard]] constexpr element_type *operator->() const noexcept { return get(); }
        [[nodiscard]] constexpr explicit operator bool() const noexcept { return static_cast<bool>(data != nullptr); }
        [[nodiscard]] constexpr explicit operator element_type *() const noexcept { return get(); }

      private:
        element_type *data = nullptr;
    };

    template <typename T>
    requires(!std::same_as<T, std::nullptr_t>) using not_null = observer_ptr<T>;

    template <typename T>
    [[nodiscard]] observer_ptr<T> make_observer(T *ptr) noexcept {
        return observer_ptr<T>(ptr);
    }

    template <typename T>
    [[nodiscard]] not_null<T> make_not_null(T *ptr) noexcept {
        return not_null<T>(ptr);
    }
}  // namespace nostd
