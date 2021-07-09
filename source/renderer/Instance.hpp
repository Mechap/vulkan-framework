#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include <cassert>
#include <string_view>
#include <vector>

#include "utility.hpp"

class Window;

class Instance final : public NoCopy, public NoMove {
  public:
    Instance(const Window &window, std::string_view app_name, uint32_t app_version = VK_MAKE_VERSION(1, 0, 0));
    ~Instance();

    [[nodiscard]] VkInstance getInstance() const { return instance; }
    [[nodiscard]] VkSurfaceKHR getSurface() const { return window_surface; }

  private:
    VkInstance createInstance(
        std::string_view app_name, std::string_view engine_name, uint32_t app_version, uint32_t engine_version, std::vector<const char *> &&required_extensions);

    VkDebugUtilsMessengerEXT createDebugMessenger();
    static void populateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT &debug_info);

    static std::vector<const char *> getRequiredExtensions();
    static bool checkValidationLayerSupport();

  private:
    VkInstance instance = nullptr;
    VkDebugUtilsMessengerEXT debug_messenger = nullptr;
    VkSurfaceKHR window_surface;
};
