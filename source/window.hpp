#pragma once

#include <GLFW/glfw3.h>

#include <glm/vec2.hpp>
#include <string_view>

struct WindowSpec {
    WindowSpec(std::string_view app_name, const glm::vec2 &window_size) : app_name(app_name), window_width(window_size.x), window_height(window_size.y) {}

    std::string app_name;

    uint32_t window_width;
    uint32_t window_height;
};

class Window final {
  public:
    Window(const WindowSpec &windowSpec);
    ~Window();

    void updateEvents() const { glfwPollEvents(); }
    [[nodiscard]] bool shouldClose() const { return glfwWindowShouldClose(window); }

    [[nodiscard]] GLFWwindow *getWindow() const { return window; }

  private:
    GLFWwindow *window = nullptr;
};
