#include "renderer/graphics/RenderPass.hpp"

#include <stdexcept>

#include "renderer/Device.hpp"
#include "renderer/Swapchain.hpp"
#include "renderer/graphics/Framebuffer.hpp"
#include "renderer/sync/CommandBuffer.hpp"

RenderPass::RenderPass(std::shared_ptr<Device> _device, std::shared_ptr<Swapchain> _swapchain) : device(std::move(_device)), swapchain(std::move(_swapchain)) {
    // color attachment
    VkAttachmentDescription color_attachment{};
    color_attachment.format = swapchain->getFormat();
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachments.push_back(color_attachment);

    // subpass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	VkAttachmentReference attachmentReference{};
	attachmentReference.attachment = 0;
	attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    subpass.colorAttachmentCount = 1; 
    subpass.pColorAttachments = &attachmentReference; 

    subpasses.push_back(subpass);

    // subpass dependency
    VkSubpassDependency dependency{};

    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // renderpass
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = subpasses.size();
    renderPassInfo.pSubpasses = subpasses.data();

    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device->getDevice(), &renderPassInfo, nullptr, &render_pass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    } else {
    	DeletionQueue::push_function([dev = device->getDevice(), rp = render_pass]() { vkDestroyRenderPass(dev, rp, nullptr); });
	}
}

void RenderPass::begin(nostd::not_null<CommandBuffer> commandBuffer, nostd::not_null<Framebuffer> framebuffer, VkClearValue clearValue) {
    VkRenderPassBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

    beginInfo.renderPass = render_pass;
    beginInfo.framebuffer = framebuffer->getFramebuffer();
    beginInfo.renderArea.offset.x = 0;
    beginInfo.renderArea.offset.y = 0;
    beginInfo.renderArea.extent = swapchain->getExtent();

    beginInfo.clearValueCount = 1;
    beginInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(commandBuffer->getCommandBuffer(), &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::end(nostd::not_null<CommandBuffer> commandBuffer) {
    vkCmdEndRenderPass(commandBuffer->getCommandBuffer());
}
