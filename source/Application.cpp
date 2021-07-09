#include "Application.hpp"

#include "config.hpp"
#include "renderer/Device.hpp"
#include "renderer/Instance.hpp"
#include "renderer/Swapchain.hpp"
#include "renderer/graphics/GraphicsPipeline.hpp"
#include "window.hpp"

Application::Application(std::string_view application_name, const glm::vec2 window_size) {
}

void Application::update() {
    while (!window->shouldClose()) {
        window->updateEvents();
    }
}
