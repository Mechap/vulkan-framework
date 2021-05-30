#include "renderer/graphics/Framebuffer.hpp"

#include <stdexcept>

#include "renderer/Device.hpp"
#include "renderer/Swapchain.hpp"
#include "renderer/graphics/RenderPass.hpp"
#include "utility.hpp"

Framebuffer::Framebuffer(const Device &device, const VkImageView &attachment, const RenderPass &renderpass, const Swapchain &extent)
    : device(device)  {
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

    framebufferInfo.renderPass = renderpass.getPass();
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &attachment;

    framebufferInfo.width = extent.getExtent().width;
    framebufferInfo.height = extent.getExtent().height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device.getDevice(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    } else {
    	DeletionQueue::push_function([fb = framebuffer, dev = device.getDevice()]() { vkDestroyFramebuffer(dev, fb, nullptr); });
	}
}

Framebuffer::Framebuffer(Framebuffer &&other) noexcept : device(other.device) {
    framebuffer = other.framebuffer;
    other.framebuffer = nullptr;
}

Framebuffer &Framebuffer::operator=(Framebuffer &&other) noexcept {
	device = other.device;

	framebuffer = other.framebuffer;
	other.framebuffer = nullptr;

	return *this;
}
