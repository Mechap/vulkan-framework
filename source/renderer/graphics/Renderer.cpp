#include "renderer/graphics/Renderer.hpp"

#include <fmt/color.h>

#include <memory>

#include "renderer/Device.hpp"
#include "renderer/Instance.hpp"
#include "renderer/Swapchain.hpp"
#include "renderer/graphics/DescriptorSetLayout.hpp"
#include "renderer/graphics/Framebuffer.hpp"
#include "renderer/graphics/GraphicsPipeline.hpp"
#include "renderer/graphics/RenderPass.hpp"
#include "renderer/graphics/Shader.hpp"
#include "renderer/graphics/ressources/Buffer.hpp"
#include "renderer/graphics/ressources/DecriptorSet.hpp"
#include "renderer/graphics/ressources/DescriptorPool.hpp"
#include "renderer/sync/CommandBuffer.hpp"
#include "renderer/sync/CommandPool.hpp"
#include "renderer/sync/Fence.hpp"
#include "renderer/sync/Semaphore.hpp"
#include "window.hpp"

namespace {
    struct FrameData {
        FrameData(const std::shared_ptr<Device> &d)
            : presentSemaphore(*d), renderSemaphore(*d), renderFence(d), commandPool(d, QueueFamilyType::GRAPHICS), commandBuffer(*d, commandPool) {}

        Semaphore presentSemaphore, renderSemaphore;
        Fence renderFence;

        CommandPool commandPool;
        CommandBuffer commandBuffer;
    };

    constexpr std::uint32_t FRAME_OVERLAP = 2;

    FrameData &getCurrentFrame(std::span<FrameData> frames, std::uint32_t frameNumber) { return frames[frameNumber % FRAME_OVERLAP]; }
}  // namespace

Renderer::Renderer(std::shared_ptr<Window> _window) {
    renderer_info.window = std::move(_window);
    renderer_info.instance = std::make_shared<Instance>(*renderer_info.window, "blank title");
    renderer_info.device = std::make_shared<Device>(renderer_info.instance);
    renderer_info.swapchain = std::make_shared<Swapchain>(renderer_info.instance, renderer_info.device, *renderer_info.window);
    renderer_info.render_pass = std::make_shared<RenderPass>(renderer_info.device, renderer_info.swapchain);
}

Renderer::~Renderer() { DeletionQueue::flush(); }

void Renderer::createGraphicsPipeline() {
    auto vertexInputDescription = std::make_unique<VertexInputDescription>(Vertex::getVertexInputDescription());

    renderer_info.graphics_pipeline = std::make_shared<GraphicsPipeline>(GraphicsPipeline::PipelineInfo(
	renderer_info.device, renderer_info.swapchain, renderer_info.render_pass, renderer_info.descriptor_set_layout, std::move(vertexInputDescription)));

    uniform_buffers.reserve(renderer_info.swapchain->getImageViewCount());
    desciptor_sets.reserve(renderer_info.swapchain->getImageViewCount());
    framebuffers.reserve(renderer_info.swapchain->getImageViewCount());

    for (std::uint32_t i = 0; i < renderer_info.swapchain->getImageViewCount(); ++i) {
        framebuffers.push_back(
            std::make_unique<Framebuffer>(renderer_info.device, *renderer_info.render_pass, renderer_info.swapchain->getImageViews()[i], renderer_info.swapchain->getExtent()));
    }
}

void Renderer::begin() { fmt::print("begin renderer\n"); }

void Renderer::draw(const Mesh &_mesh, const UniformObject &_uniform_data) {
    meshes.push_back(_mesh);
    uniforms_data.push_back(_uniform_data);
}

void Renderer::end() {
    std::vector<ShaderResource> shaderResources;
    shaderResources.emplace_back(0, ShaderResourceType::BUFFER_UNIFORM, 1, ShaderStage::VERTEX_SHADER, ShaderResourceMode::STATIC, "mvp");

    renderer_info.descriptor_set_layout = std::make_shared<DescriptorSetLayout>(renderer_info.device, shaderResources);
    createGraphicsPipeline();

    renderer_info.descritptor_pool =
        std::make_shared<DescriptorPool>(renderer_info.device, *renderer_info.descriptor_set_layout, renderer_info.swapchain->getImageViewCount() * meshes.size());

    const auto l1 = [&] {
        std::vector<Buffer> buffers{};
        buffers.reserve(meshes.size());

        for (std::uint32_t i = 0; i < meshes.size(); ++i) {
            buffers.push_back(Buffer::createUniformBuffer(sizeof(UniformObject), renderer_info.device));
        }
        return buffers;
    };

    const auto l2 = [&] {
        std::vector<DescriptorSet> ds{};
        ds.reserve(meshes.size());

        for (std::uint32_t i = 0; i < meshes.size(); ++i) {
            ds.emplace_back(renderer_info.device, renderer_info.descritptor_pool, renderer_info.descriptor_set_layout);
        }
        return ds;
    };

    for (std::uint32_t i = 0; i < renderer_info.swapchain->getImageViewCount(); ++i) {
        uniform_buffers.push_back(l1());
        desciptor_sets.emplace_back(l2());
    }

    std::array frames = {FrameData(renderer_info.device), FrameData(renderer_info.device)};
    while (!renderer_info.window->shouldClose()) {
        const auto &commandBuffer = getCurrentFrame(frames, frame_number).commandBuffer;

        renderer_info.window->updateEvents();

        getCurrentFrame(frames, frame_number).renderFence.wait(std::numeric_limits<std::uint64_t>::max());
        getCurrentFrame(frames, frame_number).renderFence.reset();

        auto swapchainImageIndex = renderer_info.swapchain->acquireNextImage(getCurrentFrame(frames, frame_number).presentSemaphore);

        commandBuffer.reset();
        commandBuffer.begin();

        VkClearValue clearValue{
            .color = {{0.f, 0.f, 0.f, 1.f}},
        };
        renderer_info.render_pass->begin(commandBuffer, *framebuffers[swapchainImageIndex], clearValue);

        renderer_info.graphics_pipeline->bind(commandBuffer);

        for (std::uint32_t i = 0; i < meshes.size(); ++i) {
            desciptor_sets[swapchainImageIndex][i].bind(*renderer_info.graphics_pipeline, commandBuffer);
            desciptor_sets[swapchainImageIndex][i].update(uniform_buffers[swapchainImageIndex][i]);

            meshes[i].bind(commandBuffer);
            uniform_buffers[swapchainImageIndex][i].update(uniforms_data[i]);

            vkCmdDrawIndexed(commandBuffer.getCommandBuffer(), static_cast<std::uint32_t>(meshes[i].getIndices().size()), 1, 0, 0, 0);
        }

        renderer_info.render_pass->end(commandBuffer);
        commandBuffer.end();

        VkSubmitInfo submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        submit.pWaitDstStageMask = &waitStages;

        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = &getCurrentFrame(frames, frame_number).presentSemaphore.getSemaphore();

        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &getCurrentFrame(frames, frame_number).renderSemaphore.getSemaphore();

        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &commandBuffer.getCommandBuffer();

        vkQueueSubmit(renderer_info.device->getQueue(QueueFamilyType::GRAPHICS), 1, &submit, getCurrentFrame(frames, frame_number).renderFence.getFence());

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &renderer_info.swapchain->getSwapchain();

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &getCurrentFrame(frames, frame_number).renderSemaphore.getSemaphore();

        presentInfo.pImageIndices = &swapchainImageIndex;

        vkQueuePresentKHR(renderer_info.device->getQueue(QueueFamilyType::PRESENT), &presentInfo);

        if (frame_number < 1) {
            frame_number++;
        } else {
            frame_number = 0;
        }
    }

    getCurrentFrame(frames, 0).renderFence.wait(std::numeric_limits<std::uint64_t>::max());
    getCurrentFrame(frames, 1).renderFence.wait(std::numeric_limits<std::uint64_t>::max());
}
