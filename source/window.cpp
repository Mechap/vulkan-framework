#include "window.hpp"

Window::Window(const WindowSpec &windowSpec) {
    glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, false);

    window = glfwCreateWindow(windowSpec.window_width, windowSpec.window_height, windowSpec.app_name.c_str(), nullptr, nullptr);
}

Window::~Window() {
	glfwDestroyWindow(window);
}
