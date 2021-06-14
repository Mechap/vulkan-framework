#include "renderer/graphics/Framebuffer.hpp"

#include <stdexcept>

#include "renderer/Device.hpp"
#include "renderer/Swapchain.hpp"
#include "renderer/graphics/RenderPass.hpp"
#include "utility.hpp"

Framebuffer::Framebuffer(std::shared_ptr<Device> _device, nostd::observer_ptr<RenderPass> renderpass, const VkImageView &attachment, const VkExtent2D &extent)
    : device(std::move(_device)) {
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

    if (renderpass) {
        framebufferInfo.renderPass = renderpass->getPass();
    } else {
    	throw std::runtime_error("renderpass ptr is invalid!");
    }

    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &attachment;

    framebufferInfo.width = extent.width;
    framebufferInfo.height = extent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device->getDevice(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    } else {
        DeletionQueue::push_function([fb = framebuffer, dev = device->getDevice()]() { vkDestroyFramebuffer(dev, fb, nullptr); });
    }
}

Framebuffer::Framebuffer(Framebuffer &&other) noexcept : device(std::move(other.device)) {
    framebuffer = other.framebuffer;
    other.framebuffer = nullptr;
}

Framebuffer &Framebuffer::operator=(Framebuffer &&other) noexcept {
    device = std::move(other.device);

    framebuffer = other.framebuffer;
    other.framebuffer = nullptr;

    return *this;
}
