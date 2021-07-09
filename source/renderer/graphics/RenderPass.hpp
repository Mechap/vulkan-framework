#pragma once

#include <vulkan/vulkan_core.h>

#include <memory>
#include <span>
#include <vector>

#include "utility.hpp"

class Device;
class Swapchain;

class Framebuffer;
class CommandBuffer;

class RenderPass final : public NoCopy, public NoMove {
  public:
    RenderPass(std::shared_ptr<Device> _device, std::shared_ptr<Swapchain> _swapchain);
    ~RenderPass();

    void begin(const CommandBuffer &commandBuffer, const Framebuffer &framebuffer, VkClearValue clearValue);
    void end(const CommandBuffer &commandBuffer);

  public:
    [[nodiscard]] const VkRenderPass &getPass() const { return render_pass; }
    [[nodiscard]] std::span<const VkSubpassDescription> getSubpasses() const { return subpasses; }

    [[nodiscard]] std::size_t getAttachmentCount() const { return attachments.size(); }
    [[nodiscard]] std::span<const VkAttachmentDescription> getAttachments() const { return attachments; }

  private:
    std::shared_ptr<Device> device;
    std::shared_ptr<Swapchain> swapchain;

    VkRenderPass render_pass = nullptr;

    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkSubpassDescription> subpasses;
};
