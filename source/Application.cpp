#include "Application.hpp"

#include "config.hpp"
#include "renderer/Device.hpp"
#include "renderer/Instance.hpp"
#include "renderer/Swapchain.hpp"
#include "renderer/graphics/GraphicsPipeline.hpp"
#include "window.hpp"

Application::Application(const std::string_view application_name, const Vector2i window_size) {
    /*
        window = std::make_unique<Window>(WindowSpec{application_name, static_cast<uint32_t>(window_size.x), static_cast<uint32_t>(window_size.y)});
            instance = std::make_unique<Instance>(*window, application_name);
            device = std::make_unique<Device>(*instance);
            swapchain = std::make_unique<Swapchain>(*device, *instance, *window);
            */
}

void Application::update() {
    while (!window->shouldClose()) {
        window->updateEvents();
    }
}
