#pragma once

#include <GLFW/glfw3.h>

#include <string_view>

#include "math/Vector2.hpp"

struct WindowSpec {
    WindowSpec(const std::string_view app_name, const Vector2<uint32_t> &window_size) : app_name(app_name), window_width(window_size.x), window_height(window_size.y) {}

    std::string app_name;

    uint32_t window_width;
    uint32_t window_height;
};

class Window final {
  public:
    Window(const WindowSpec &windowSpec);
    ~Window();

    void updateEvents() const { glfwPollEvents(); }
    bool shouldClose() const { return glfwWindowShouldClose(window); }

    GLFWwindow *getWindow() const { return window; }

  private:
    GLFWwindow *window = nullptr;
};
