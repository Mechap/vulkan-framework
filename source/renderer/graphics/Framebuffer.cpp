#include "renderer/graphics/Framebuffer.hpp"

#include <stdexcept>

#include "renderer/Device.hpp"
#include "renderer/Swapchain.hpp"
#include "renderer/graphics/RenderPass.hpp"

Framebuffer::Framebuffer(const Device &device, const VkImageView &attachment, const RenderPass &renderpass, const Vector2u &window_size)
    : device(device), window_size(window_size) {
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

    framebufferInfo.renderPass = renderpass.getPass();
    framebufferInfo.attachmentCount = renderpass.getAttachmentCount();
    framebufferInfo.pAttachments = &attachment;

    framebufferInfo.width = window_size.x;
    framebufferInfo.height = window_size.y;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device.getDevice(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }
}

Framebuffer::~Framebuffer() {
    if (framebuffer != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(device.getDevice(), framebuffer, nullptr);
    }
}
