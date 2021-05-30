#pragma once

#include <vulkan/vulkan_core.h>

#include <span>
#include <vector>

#include "utility.hpp"

class Device;
class Swapchain;

class Framebuffer;
class CommandBuffer;

class RenderPass final : public NoCopy, public NoMove {
  public:
    RenderPass(const Device &device, const Swapchain &swapchain);

    void begin(const CommandBuffer &commandBuffer, const Framebuffer &framebuffer, const VkClearValue clearValue);
	void end(const CommandBuffer &commandBuffer);

  public:
    const VkRenderPass &getPass() const { return render_pass; }
    std::span<const VkSubpassDescription> getSubpasses() const { return subpasses; }

    std::size_t getAttachmentCount() const { return attachments.size(); }
    std::span<const VkAttachmentDescription> getAttachments() const { return attachments; }

  private:
    const Device &device;
	const Swapchain &swapchain;

    VkRenderPass render_pass = nullptr;

    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkSubpassDescription> subpasses;
};

