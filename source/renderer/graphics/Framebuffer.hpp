#pragma once

#include <vulkan/vulkan_core.h>

#include "math/Vector2.hpp"

class Device;
class Swapchain;

class RenderPass;

class Framebuffer {
  public:
    Framebuffer(const Device &device, const VkImageView &attachment, const RenderPass &renderpass, const Vector2u &window_size);
    ~Framebuffer();

    const VkFramebuffer &getFramebuffer() const { return framebuffer; }
    const Vector2u &getWindowSize() const { return window_size; }

  private:
    const Device &device;
    const Vector2u window_size;

    VkFramebuffer framebuffer = nullptr;
};
