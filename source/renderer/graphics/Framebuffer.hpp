#pragma once

#include <vulkan/vulkan_core.h>

#include "math/Vector2.hpp"

class Device;
class Swapchain;

class RenderPass;

class Framebuffer {
  public:
    Framebuffer(const Device &device, const VkImageView &attachment, const RenderPass &renderpass, const Swapchain &extent);

	Framebuffer(Framebuffer &&other) noexcept;
	Framebuffer &operator=(Framebuffer &&other) noexcept;

    const VkFramebuffer &getFramebuffer() const { return framebuffer; }

  private:
	std::reference_wrapper<const Device> device;
    VkFramebuffer framebuffer = nullptr;
};
