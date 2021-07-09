#pragma once

#include <glm/vec2.hpp>
#include <memory>
#include <string_view>

class Window;
class Instance;
class Device;
class Swapchain;

class Application {
  public:
    Application(std::string_view application_name, const glm::vec2 window_size = {800, 600});

    void update();

  private:
    std::unique_ptr<Window> window;
};
