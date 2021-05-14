#include "renderer/graphics/Framebuffer.hpp"

#include <stdexcept>

#include "renderer/Device.hpp"
#include "renderer/Swapchain.hpp"
#include "renderer/graphics/RenderPass.hpp"
#include "utility.hpp"

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
    } else {
    	DeletionQueue::push_function([fb = framebuffer, dev = device.getDevice()]() { vkDestroyFramebuffer(dev, fb, nullptr); });
	}
}

Framebuffer::~Framebuffer() {
    //DeletionQueue::push_function([fb = framebuffer, dev = device.getDevice()]() { vkDestroyFramebuffer(dev, fb, nullptr); });
}

Framebuffer::Framebuffer(Framebuffer &&other) noexcept : device(std::move(other.device)) {
    framebuffer = other.framebuffer;
    other.framebuffer = nullptr;
}
