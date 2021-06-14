#pragma once

#include <vulkan/vulkan_core.h>

#include <functional>

#include "utility.hpp"

class Device;
class Swapchain;

class RenderPass;

class Framebuffer {
  public:
    Framebuffer(std::shared_ptr<Device> _device, nostd::observer_ptr<RenderPass> renderpass, const VkImageView &attachment, const VkExtent2D &extent);

    Framebuffer(Framebuffer &&other) noexcept;
    Framebuffer &operator=(Framebuffer &&other) noexcept;

    [[nodiscard]] const VkFramebuffer &getFramebuffer() const { return framebuffer; }

  private:
    std::shared_ptr<Device> device;
    VkFramebuffer framebuffer = nullptr;
};
