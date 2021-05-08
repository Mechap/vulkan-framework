#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

#include <span>
#include <vector>

#include "utility.hpp"

class Device;
class Instance;

class Window;

class Semaphore;

struct SwapchainProperties {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

class Swapchain final : public NoCopy, public NoMove {
  public:
    Swapchain(const Device &device, const Instance &instance, const Window &window);
    ~Swapchain();

    [[nodiscard]] const VkSwapchainKHR &getSwapchain() const { return swapchain; }
    [[nodiscard]] const VkFormat &getFormat() const { return swapchain_image_format; }
    [[nodiscard]] const VkExtent2D &getExtent() const { return swapchain_extent; }

    [[nodiscard]] std::size_t getImageViewCount() const { return image_views.size(); }
    [[nodiscard]] std::span<const VkImageView> getImageViews() const { return image_views; }

    uint32_t acquireNextImage(const Semaphore &presentSemaphore) const;

  private:
    VkSwapchainKHR create(GLFWwindow *window);
    void createViews();

  private:
    VkSwapchainKHR swapchain = nullptr;

    const VkSurfaceKHR &surface;
    const Device &device;

    SwapchainProperties properties;
    VkFormat swapchain_image_format;
    VkExtent2D swapchain_extent;

    uint32_t image_count;
    std::vector<VkImage> images;
    std::vector<VkImageView> image_views;
};
