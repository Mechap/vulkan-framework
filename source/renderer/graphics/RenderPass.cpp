#include "renderer/graphics/RenderPass.hpp"

#include <stdexcept>

#include "renderer/Device.hpp"
#include "renderer/Swapchain.hpp"
#include "renderer/graphics/Framebuffer.hpp"
#include "renderer/sync/CommandBuffer.hpp"

RenderPass::RenderPass(const Device &device, const Swapchain &swapchain, const CommandBuffer &commandBuffer) : device(device), command_buffer(commandBuffer) {
    // color attachment
    VkAttachmentDescription color_attachment{};
    color_attachment.format = swapchain.getFormat();
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachments.push_back(color_attachment);

    // subpass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    std::vector<VkAttachmentReference> attachmentReferences;
    attachmentReferences.emplace_back(VkAttachmentReference{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});

    subpass.colorAttachmentCount = attachmentReferences.size();
    subpass.pColorAttachments = attachmentReferences.data();

    subpasses.push_back(subpass);

    // subpass dependency
	/*
    VkSubpassDependency dependency{};

    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	*/

    // renderpass
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = subpasses.size();
    renderPassInfo.pSubpasses = subpasses.data();

    // renderPassInfo.dependencyCount = 1;
    // renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device.getDevice(), &renderPassInfo, nullptr, &render_pass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

RenderPass::~RenderPass() {
    DeletionQueue::push_function([=]() { vkDestroyRenderPass(device.getDevice(), render_pass, nullptr); });
}

void RenderPass::begin(const Framebuffer &framebuffer, const VkClearValue &clearValue) {
    VkRenderPassBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

    beginInfo.renderPass = render_pass;
    beginInfo.framebuffer = framebuffer.getFramebuffer();
    beginInfo.renderArea.offset = {0, 0};
    beginInfo.renderArea.extent = {framebuffer.getWindowSize().x, framebuffer.getWindowSize().y};
    beginInfo.clearValueCount = 1;
    beginInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(command_buffer.getCommandBuffer(), &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::end() { vkCmdEndRenderPass(command_buffer.getCommandBuffer()); }
