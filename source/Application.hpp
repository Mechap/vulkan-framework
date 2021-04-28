#pragma once

#include <memory>
#include <string_view>

#include "math/Vector2.hpp"

class Window;
class Instance;
class Device;
class Swapchain;

class Application {
  public:
    Application(const std::string_view application_name, const Vector2i window_size = {800, 600});

	void update();

  private:
    std::unique_ptr<Window> window;
    std::unique_ptr<Instance> instance;
    std::unique_ptr<Device> device;
    std::unique_ptr<Swapchain> swapchain;
};
